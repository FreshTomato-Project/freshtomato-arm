/*
 * uqmid
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * based on netifd/ubus.c by
 * Copyright (C) 2012 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "gsmtap_util.h"
#include "osmocom/fsm.h"
#include "qmi-enums-wds.h"

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <libubus.h>
#include <talloc.h>

#include "uqmid.h"
#include "logging.h"
#include "modem.h"
#include "utils.h"

#include "ubus.h"

struct ubus_context *ubus_ctx = NULL;
static struct blob_buf b;
static const char *ubus_path;
/* global object */

static void uqmid_ubus_add_fd(void)
{
	ubus_add_uloop(ubus_ctx);
	system_fd_set_cloexec(ubus_ctx->sock.fd);
}

static int uqmid_handle_restart(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
				const char *method, struct blob_attr *msg)
{
	/* TODO: netifd is calling itself via exec, unsure if we need this also.
	uqmid_restart(); */
	return 0;
}

static int uqmid_reload(void)
{
	return 1;
}

static int uqmid_handle_reload(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			       const char *method, struct blob_attr *msg)
{
	if (uqmid_reload())
		return UBUS_STATUS_NOT_FOUND;

	return UBUS_STATUS_OK;
}

enum {
	GSMTAP_TARGET,
	__GSMTAP_MAX
};

static const struct blobmsg_policy enable_gsmtap_policy[__GSMTAP_MAX] = {
	[GSMTAP_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
};

static int enable_gsmtap(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			 const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__GSMTAP_MAX];
	char *target = NULL;
	int ret;

	blobmsg_parse(enable_gsmtap_policy, __GSMTAP_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[GSMTAP_TARGET]) {
		target = blobmsg_get_string(tb[GSMTAP_TARGET]);
	} else {
		target = "127.0.0.1";
	}
	ret = gsmtap_enable(target);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

static int disable_gsmtap_policy(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
				 const char *method, struct blob_attr *msg)
{
	gsmtap_disable();
	return UBUS_STATUS_OK;
}

static int uqmid_add_object(struct ubus_object *obj)
{
	int ret = ubus_add_object(ubus_ctx, obj);

	if (ret != 0)
		LOG_ERROR("Failed to publish object '%s': %s\n", obj->name, ubus_strerror(ret));

	return ret;
}

enum {
	AM_NAME,
	AM_DEVICE,
	AM_DRIVER,
	__AM_MAX
};

/** ubus call add_modem{name: slot1, device: /dev/cdc-wdm0, driver: qmi or mbim (optional, default qmi)}` */
static const struct blobmsg_policy add_modem_policy[__AM_MAX] = {
	[AM_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[AM_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
	[AM_DRIVER] = { .name = "driver", .type = BLOBMSG_TYPE_STRING },
};

static int add_modem(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		     const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__AM_MAX];
	char *name = NULL;
	char *device = NULL;
	/* tunnel qmi over mbim? */
	bool qmi_over_mbim = false;
	int ret;

	blobmsg_parse(add_modem_policy, __AM_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[AM_NAME]) {
		LOG_ERROR("ubus add_modem: missing required argument name.");
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	name = blobmsg_get_string(tb[AM_NAME]);

	if (!tb[AM_DEVICE]) {
		LOG_ERROR("ubus add_modem: missing required argument device.");
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	device = blobmsg_get_string(tb[AM_DEVICE]);

	if (tb[AM_DRIVER]) {
		const char *driver = blobmsg_get_string(tb[AM_DRIVER]);
		if (!strcmp(driver, "qmi")) {
			/* default */
		} else if (!strcmp(driver, "mbim")) {
			qmi_over_mbim = true;
		} else {
			LOG_ERROR("ubus add_modem: invalid driver. Valid qmi or mbim.");
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
	}

	ret = uqmid_modem_add(name, device, qmi_over_mbim);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

/** ubus call remove_modem{name: slot1}` */
enum {
	RM_NAME,
	__RM_MAX,
};

static const struct blobmsg_policy remove_modem_policy[__RM_MAX] = {
	[RM_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
};

static int remove_modem(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__RM_MAX];
	struct modem *modem;
	char *name = NULL;
	int ret;

	blobmsg_parse(remove_modem_policy, __RM_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[RM_NAME]) {
		LOG_ERROR("ubus add_modem: missing required argument name.");
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	name = blobmsg_get_string(tb[RM_NAME]);
	modem = uqmid_modem_find_by_name(name);
	if (!modem) {
		LOG_ERROR("Couldn't find modem %s.", name);
		return UBUS_STATUS_NOT_FOUND;
	}

	ret = uqmid_modem_remove(modem);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

/** clean up the ubus state of a modem */
void uqmid_ubus_modem_destroy(struct modem *modem)
{
	ubus_remove_object(ubus_ctx, &modem->ubus);
}

/** inform ubus subscribers of a state change of a modem */
void uqmid_ubus_modem_notify_change(struct modem *modem, int event)
{
	/* TODO: */
}

static void uqmid_ubus_reconnect_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry = {
		.cb = uqmid_ubus_reconnect_timer,
	};
	int t = 2;

	if (ubus_reconnect(ubus_ctx, ubus_path) != 0) {
		LOG_INFO("failed to reconnect, trying again in %d seconds\n", t);
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	LOG_INFO("reconnected to ubus, new id: %08x\n", ubus_ctx->local_id);
	uqmid_ubus_add_fd();
}

static void uqmid_ubus_connection_lost(struct ubus_context *ctx)
{
	uqmid_ubus_reconnect_timer(NULL);
}

static struct ubus_method main_object_methods[] = {
	{ .name = "restart", .handler = uqmid_handle_restart },
	{ .name = "reload", .handler = uqmid_handle_reload },
	UBUS_METHOD("enable_gsmtap", enable_gsmtap, enable_gsmtap_policy),
	{ .name = "disable_gsmtap", .handler = disable_gsmtap_policy },
	UBUS_METHOD("add_modem", add_modem, add_modem_policy),
	UBUS_METHOD("remove_modem", remove_modem, remove_modem_policy),
};

static struct ubus_object_type main_object_type = UBUS_OBJECT_TYPE("uqmid", main_object_methods);

static struct ubus_object main_object = {
	.name = "uqmid",
	.type = &main_object_type,
	.methods = main_object_methods,
	.n_methods = ARRAY_SIZE(main_object_methods),
};

/* modem object */

/** ubus call uqmid.modem.some1 remove_modem{name: slot1}` */
static int modem_remove(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			const char *method, struct blob_attr *msg)
{
	struct modem *modem = container_of(obj, struct modem, ubus);
	int ret;

	ret = uqmid_modem_remove(modem);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

enum { CFG_APN, CFG_PIN, CFG_PUK, CFG_ROAMING, CFG_USERNAME, CFG_PASSWORD, __CFG_MAX };

/** ubus call modem_configure{apn: internet, pin: 2342, roaming: false}` */
static const struct blobmsg_policy modem_configure_policy[__CFG_MAX] = {
	[CFG_APN] = { .name = "apn", .type = BLOBMSG_TYPE_STRING },
	[CFG_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_STRING },
	[CFG_PUK] = { .name = "puk", .type = BLOBMSG_TYPE_STRING },
	[CFG_ROAMING] = { .name = "roaming", .type = BLOBMSG_TYPE_BOOL },
	[CFG_USERNAME] = { .name = "username", .type = BLOBMSG_TYPE_STRING },
	[CFG_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
};

static int modem_configure(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			   const char *method, struct blob_attr *msg)
{
	struct modem *modem = container_of(obj, struct modem, ubus);
	struct blob_attr *tb[__CFG_MAX];
	char *value;
	int ret;

	/* prevent mixing previous configure calls */
	memset(&modem->config, 0, sizeof(struct modem_config));
	blobmsg_parse(modem_configure_policy, __CFG_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[CFG_APN]) {
		TALLOC_FREE(modem->config.apn);
		value = blobmsg_get_string(tb[CFG_APN]);
		if (value && strlen(value))
			modem->config.apn = talloc_strdup(modem, blobmsg_get_string(tb[CFG_APN]));
	}

	if (tb[CFG_PIN]) {
		TALLOC_FREE(modem->config.pin);
		value = blobmsg_get_string(tb[CFG_PIN]);
		if (value && strlen(value))
			modem->config.pin = talloc_strdup(modem, blobmsg_get_string(tb[CFG_PIN]));
	}

	if (tb[CFG_PUK]) {
		TALLOC_FREE(modem->config.puk);
		value = blobmsg_get_string(tb[CFG_PUK]);
		if (value && strlen(value))
			modem->config.puk = talloc_strdup(modem, blobmsg_get_string(tb[CFG_PUK]));
	}

	if (tb[CFG_ROAMING]) {
		modem->config.roaming = blobmsg_get_bool(tb[CFG_ROAMING]);
	}

	if (tb[CFG_USERNAME]) {
		TALLOC_FREE(modem->config.username);
		value = blobmsg_get_string(tb[CFG_USERNAME]);
		if (value && strlen(value))
			modem->config.username = talloc_strdup(modem, blobmsg_get_string(tb[CFG_USERNAME]));
	}

	if (tb[CFG_PASSWORD]) {
		TALLOC_FREE(modem->config.password);
		value = blobmsg_get_string(tb[CFG_PASSWORD]);
		if (value && strlen(value))
			modem->config.password = talloc_strdup(modem, blobmsg_get_string(tb[CFG_PASSWORD]));
	}

	modem->config.pdp_type = QMI_WDS_PDP_TYPE_IPV4;
	modem->config.configured = true;

	ret = uqmid_modem_configured(modem);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

static void modem_get_opmode_cb(void *data, int opmode_err, int opmode)
{
	struct ubus_request_data *req = data;
	if (opmode_err) {
		blob_buf_init(&b, 0);
		blobmsg_add_u16(&b, "operror", opmode_err & 0xff);
	} else {
		blob_buf_init(&b, 0);
		blobmsg_add_u16(&b, "opmode", opmode & 0xff);
	}
	/* TODO: add additional opmode as str */
	ubus_send_reply(ubus_ctx, req, b.head);
	ubus_complete_deferred_request(ubus_ctx, req, 0);
	free(req);
}

static int modem_get_opmode(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			    const char *method, struct blob_attr *msg)
{
	struct modem *modem = container_of(obj, struct modem, ubus);
	int ret;
	struct ubus_request_data *new_req = calloc(sizeof(*req), 1);
	if (!new_req)
		return UBUS_STATUS_NO_MEMORY;

	ubus_defer_request(ctx, req, new_req);
	ret = uqmid_modem_get_opmode(modem, modem_get_opmode_cb, new_req);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

static int modem_networkstatus(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			       const char *method, struct blob_attr *msg)
{
	struct modem *modem = container_of(obj, struct modem, ubus);
	int ret;

	ret = uqmid_modem_networkstatus(modem);
	if (ret)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return UBUS_STATUS_OK;
}

static void blob_add_addr(struct blob_buf *blob, const char *name, struct sockaddr *addr)
{
	struct sockaddr_in *v4 = (struct sockaddr_in *)addr;
	struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)addr;
	char buf[INET6_ADDRSTRLEN];

	if (!addr) {
		blobmsg_add_string(blob, name, "");
		return;
	}

	switch (addr->sa_family) {
	case AF_INET:
		blobmsg_add_string(blob, name, inet_ntop(AF_INET, &v4->sin_addr, buf, sizeof(buf)));
		break;
	case AF_INET6:
		blobmsg_add_string(blob, name, inet_ntop(AF_INET6, &v6->sin6_addr, buf, sizeof(buf)));
		break;
	}
}

#define BLOBMSG_ADD_STR_CHECK(buffer, field, value) blobmsg_add_string(buffer, field, value ? value : "")

static int modem_dump_state(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			    const char *method, struct blob_attr *msg)
{
	struct modem *modem = container_of(obj, struct modem, ubus);

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "name", modem->name);
	blobmsg_add_string(&b, "device", modem->device);
	blobmsg_add_u32(&b, "state", modem->fi->state);
	blobmsg_add_string(&b, "state_name",  osmo_fsm_inst_state_name(modem->fi));
	BLOBMSG_ADD_STR_CHECK(&b, "manufactor", modem->manuf);
	BLOBMSG_ADD_STR_CHECK(&b, "model", modem->model);
	BLOBMSG_ADD_STR_CHECK(&b, "rev", modem->rev);
	BLOBMSG_ADD_STR_CHECK(&b, "imei", modem->imei);
	BLOBMSG_ADD_STR_CHECK(&b, "imeisv", modem->imeisv);
	BLOBMSG_ADD_STR_CHECK(&b, "meid", modem->meid);
	BLOBMSG_ADD_STR_CHECK(&b, "imsi", modem->imsi);
	BLOBMSG_ADD_STR_CHECK(&b, "iccid", modem->iccid);
	/* session state */
	BLOBMSG_ADD_STR_CHECK(&b, "config_apn", modem->config.apn);
	blobmsg_add_u8(&b, "roaming", modem->config.roaming);
	blob_add_addr(&b, "ipv4_addr", (struct sockaddr *)&modem->brearer.v4_addr);
	blob_add_addr(&b, "ipv4_netmask", (struct sockaddr *)&modem->brearer.v4_netmask);
	blob_add_addr(&b, "ipv4_gateway", (struct sockaddr *)&modem->brearer.v4_gateway);
	blob_add_addr(&b, "ipv6", (struct sockaddr *)&modem->brearer.v6);
	blob_add_addr(&b, "dns1", (struct sockaddr *)&modem->brearer.dns1);
	blob_add_addr(&b, "dns2", (struct sockaddr *)&modem->brearer.dns2);
	/* sim */
	/* TODO: add human readable enum values */
	blobmsg_add_u16(&b, "sim_state", modem->sim.state);
	blobmsg_add_string(&b, "sim_state_name",  osmo_fsm_inst_state_name(modem->sim.fi));
	blobmsg_add_u8(&b, "sim_use_upin", modem->sim.use_upin);
	blobmsg_add_u16(&b, "sim_pin_retries", modem->sim.pin_retries & 0xff);
	blobmsg_add_u16(&b, "sim_puk_retries", modem->sim.puk_retries & 0xff);

	ubus_send_reply(ubus_ctx, req, b.head);
	return UBUS_STATUS_OK;
}

static struct ubus_method modem_instance_object_methods[] = {
	UBUS_METHOD("configure", modem_configure, modem_configure_policy),
	UBUS_METHOD_NOARG("remove", modem_remove),
	UBUS_METHOD_NOARG("opmode", modem_get_opmode),
	UBUS_METHOD_NOARG("networkstatus", modem_networkstatus),
	UBUS_METHOD_NOARG("dump", modem_dump_state),
	//	{ .name = "serving_system", .handler = modem_get_serving_system},
};

static struct ubus_object_type modem_instance_object_type =
	UBUS_OBJECT_TYPE("uqmid_modem_instance", modem_instance_object_methods);

/* TODO: rename modem, modem instance */
static struct ubus_method modem_object_methods[] = {};

static struct ubus_object_type modem_object_type = UBUS_OBJECT_TYPE("uqmid_modem", modem_object_methods);

/* empty uqmid.modem object, only used as paraent */
static struct ubus_object modem_object = {
	.name = "uqmid.modem",
	.type = &modem_object_type,
	.methods = modem_object_methods,
	.n_methods = 0,
};

/** called by modem.c to create the ubus object for a modem */
int uqmid_ubus_modem_add(struct modem *modem)
{
	struct ubus_object *obj = &modem->ubus;
	int ret = 0;

	obj->name = talloc_asprintf(modem, "%s.modem.%s", main_object.name, modem->name);
	obj->type = &modem_instance_object_type;
	obj->methods = modem_instance_object_methods;
	obj->n_methods = ARRAY_SIZE(modem_instance_object_methods);
	if ((ret = ubus_add_object(ubus_ctx, &modem->ubus))) {
		LOG_ERROR("failed to publish ubus object for modem '%s' (ret = %d).\n", modem->name, ret);
		talloc_free((void *)obj->name);
		obj->name = NULL;
		return ret;
	}

	return ret;
}

int uqmid_ubus_init(const char *path)
{
	int ret;

	ubus_path = path;
	ubus_ctx = ubus_connect(path);
	if (!ubus_ctx)
		return -EIO;

	LOG_DEBUG("connected as %08x\n", ubus_ctx->local_id);
	ubus_ctx->connection_lost = uqmid_ubus_connection_lost;
	uqmid_ubus_add_fd();

	ret = uqmid_add_object(&main_object);
	if ((ret = uqmid_add_object(&modem_object))) {
		fprintf(stderr, "Failed to add modem object %d", ret);
		return ret;
	}

	return 0;
}
