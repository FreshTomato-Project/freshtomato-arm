/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

/* Used by uqmid to contain the modem state */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <talloc.h>

#include "qmi-message.h"

#include "qmi-enums-dms.h"
#include "qmi-enums-wds.h"
#include "qmi-enums.h"
#include "sim_fsm.h"
#include "uqmid.h"
#include "qmi-errors.h"
#include "qmi-errors.c"

#include "mbim.h"
#include "services.h"

#include "modem.h"
#include "modem_fsm.h"
#include "modem_tx.h"
#include "osmocom/fsm.h"
#include "ubus.h"
#include "logging.h"
#include "utils.h"

LIST_HEAD(uqmid_modems);

struct modem *
uqmid_modem_find_by_device(const char *device)
{
	struct modem *modem;

	list_for_each_entry(modem, &uqmid_modems, list) {
		if (!strcmp(device, modem->device))
			return modem;
	}

	return NULL;
}

struct modem *
uqmid_modem_find_by_name(const char *name)
{
	struct modem *modem;
	list_for_each_entry(modem, &uqmid_modems, list) {
		if (!strcmp(name, modem->name))
			return modem;
	}

	return NULL;
}

static void
modem_error_cb(struct qmi_dev *dev, void *data)
{
	struct modem *modem = data;

	osmo_fsm_inst_term(modem->fi, OSMO_FSM_TERM_ERROR, modem);
	uqmid_ubus_modem_destroy(modem);
	qmi_device_close(modem->qmi, 0);

	list_del(&modem->list);
	talloc_free(modem);
}

/**
 *
 * @param name an unique name also used by ubus
 * @param device path to the device. /dev/cdc-wdm0
 * @param qmi_over_mbim true if qmi needs to be tunneled over mbim
 * @return 0 on success
 */
int
uqmid_modem_add(const char *name, const char *device, bool qmi_over_mbim)
{
	struct modem *modem = uqmid_modem_find_by_name(name);
	int ret;
	if (modem)
		return -EALREADY;

	modem = talloc_zero(NULL, struct modem);
	modem->name = talloc_strdup(modem, name);
	modem->device = talloc_strdup(modem, device);
	modem->qmi = qmi_device_open(modem, device);
	if (!modem->qmi)
		goto err;

	ret = uqmid_ubus_modem_add(modem);
	if (ret)
		goto err;

	modem->qmi->error_cb = modem_error_cb;
	modem->qmi->error_cb_data = modem;
	modem->sim.use_uim = true;

	modem->fi = modem_fsm_alloc(modem);
	modem->sim.fi = sim_fsm_alloc(modem);
	list_add(&modem->list, &uqmid_modems);
	modem_fsm_start(modem);

	return 0;

err:
	if (modem->qmi)
		qmi_device_close(modem->qmi, 0);

	talloc_free(modem);
	return -1;
}


static void
modem_remove_cb(struct qmi_dev *dev, void *data)
{
	struct modem *modem = data;

	osmo_fsm_inst_term(modem->fi, OSMO_FSM_TERM_REGULAR, modem);
	uqmid_ubus_modem_destroy(modem);

	list_del(&modem->list);
	talloc_free(modem);
}

int
uqmid_modem_remove(struct modem *modem)
{
	if (!modem)
		return -EINVAL;

	if (!modem->fi)
		return 0;

	if (modem->fi->state == MODEM_ST_DESTROY)
		return 0;

	if (modem->fi->proc.terminating)
		return 0;

	/* TODO: move the assignment of closing_cb */
	modem->qmi->closing_cb = modem_remove_cb;
	modem->qmi->closing_cb_data = modem;
	osmo_fsm_inst_dispatch(modem->fi, MODEM_EV_REQ_DESTROY, modem);

	return 0;
}

int
uqmid_modem_configured(struct modem *modem)
{
	osmo_fsm_inst_dispatch(modem->fi, MODEM_EV_REQ_CONFIGURED, NULL);
	return 0;
}

static void
wds_get_packet_status_cb(struct qmi_service *service, struct qmi_request *req, struct qmi_msg *msg)
{
	struct modem *modem = req->cb_data;
	int ret = 0;
	struct qmi_wds_get_packet_service_status_response res = {};

	if (req->ret) {
		modem_log(modem, LOGL_INFO, "Failed to get operating mode. Status %d/%s.", req->ret, qmi_get_error_str(req->ret));
		/* FIXME: send ubus failed */
		return;
	}

	ret = qmi_parse_wds_get_packet_service_status_response(msg, &res);
	if (ret) {
		modem_log(modem, LOGL_INFO, "Failed to get packet service status. Failed to parse message");
		/* FIXME: send ubus failed */
		return;
	}

	if (!res.set.connection_status) {
		modem_log(modem, LOGL_INFO, "Failed to get packet service status. No connection status");
	}

	modem_log(modem, LOGL_INFO, "Conn Status is %d", res.data.connection_status);

	/* Fix send back */
	return;
}

struct modem_opmode_data {
	struct modem *modem;
	void *cb_data;
	uqmid_modem_get_opmode_cb cb;
};

static void
modem_get_opmode_cb(struct qmi_service *service, struct qmi_request *req, struct qmi_msg *msg)
{
	struct modem_opmode_data *data= req->cb_data;
	struct modem *modem = data->modem;
	int ret = 0;
	struct qmi_dms_get_operating_mode_response res = {};

	if (!data) {
		return;
	}

	if (req->ret) {
		modem_log(modem, LOGL_INFO, "Failed to get operating mode. Status %d/%s.", req->ret, qmi_get_error_str(req->ret));
		goto out;
	}

	ret = qmi_parse_dms_get_operating_mode_response(msg, &res);
	if (ret) {
		modem_log(modem, LOGL_INFO, "Failed to get operating mode. Failed to parse message");
		goto out;
	}

	if (!res.set.mode) {
		modem_log(modem, LOGL_INFO, "Failed to get operating mode. Failed to parse message");
	}

	modem_log(modem, LOGL_INFO, "Opmode is %d", res.data.mode);
	data->cb(data->cb_data, 0, res.data.mode);
out:
	free(data);
	return;
}

int
uqmid_modem_get_opmode(struct modem *modem, uqmid_modem_get_opmode_cb cb, void *cb_data)
{
	struct qmi_service *dms = uqmi_service_find(modem->qmi, QMI_SERVICE_DMS);
	if (!dms) {
		cb(cb_data, -ENOENT, 0);
		modem_log(modem, LOGL_ERROR, "Ubus call requested, but dms not available.");
		return -1;
	}

	struct modem_opmode_data *data = calloc(1, sizeof(*data));
	if (!data) {
		cb(cb_data, -ENOMEM, 0);
		return -1;
	}

	data->cb = cb;
	data->cb_data = cb_data;
	data->modem = modem;

	uqmi_service_send_simple(dms, qmi_set_dms_get_operating_mode_request, modem_get_opmode_cb, data);
	return 0;
}

int uqmid_modem_networkstatus(struct modem *modem)
{
	struct qmi_service *wds = uqmi_service_find(modem->qmi, QMI_SERVICE_WDS);
	if (!wds) {
		modem_log(modem, LOGL_ERROR, "Ubus call requested, but wds not available.");
		return -1;
	}

	uqmi_service_send_simple(wds, qmi_set_wds_get_packet_service_status_request, wds_get_packet_status_cb, modem);
	return 0;
}

void uqmid_modem_set_error(struct modem *modem, const char *error)
{
	if (modem->state.error) {
		talloc_free(modem->state.error);
	}

	if (error) {
		modem_log(modem, LOGL_ERROR, "failed with %s", error);
		modem->state.error = talloc_strdup(modem, error);
	} else {
		modem->state.error = NULL;
	}
}
