/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
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

#include <stdlib.h>
#include <arpa/inet.h>

#include "qmi-message.h"

static const struct {
	const char *auth_name;
	QmiWdsAuthentication auth;
} auth_modes[] = {
	{ "none", QMI_WDS_AUTHENTICATION_NONE },
	{ "pap", QMI_WDS_AUTHENTICATION_PAP },
	{ "chap", QMI_WDS_AUTHENTICATION_CHAP },
	{ "both", QMI_WDS_AUTHENTICATION_PAP | QMI_WDS_AUTHENTICATION_CHAP },
};

static const struct {
	const char *ipfam_name;
	const QmiWdsIpFamily mode;
} ipfam_modes[] = {
	{ "ipv4", QMI_WDS_IP_FAMILY_IPV4 },
	{ "ipv6", QMI_WDS_IP_FAMILY_IPV6 },
	{ "unspecified", QMI_WDS_IP_FAMILY_UNSPECIFIED },
};

static const struct {
	const char *pdp_name;
	const QmiWdsPdpType type;
} pdp_types[] = {
	{ "ipv4", QMI_WDS_PDP_TYPE_IPV4 },
	{ "ppp", QMI_WDS_PDP_TYPE_PPP },
	{ "ipv6", QMI_WDS_PDP_TYPE_IPV6 },
	{ "ipv4v6", QMI_WDS_PDP_TYPE_IPV4_OR_IPV6 },
};

static const struct {
	const char *profile_name;
	const QmiWdsProfileType profile;
} profile_types[] = {
	{ "3gpp", QMI_WDS_PROFILE_TYPE_3GPP },
	{ "3gpp2", QMI_WDS_PROFILE_TYPE_3GPP2 },
};

struct uqmi_wds_profile_identifier {
	QmiWdsProfileType type;
	uint32_t index;
};

static struct qmi_wds_start_network_request wds_sn_req = {
	QMI_INIT(authentication_preference,
	         QMI_WDS_AUTHENTICATION_PAP | QMI_WDS_AUTHENTICATION_CHAP),
};

static struct qmi_wds_stop_network_request wds_stn_req;

static struct qmi_wds_modify_profile_request wds_mp_req = {
	QMI_INIT_SEQUENCE(profile_identifier,
		.profile_type = QMI_WDS_PROFILE_TYPE_3GPP,
		.profile_index = 1,
	),
	QMI_INIT(apn_disabled_flag, false),
};

static struct qmi_wds_create_profile_request wds_cp_req = {
	QMI_INIT(profile_type,QMI_WDS_PROFILE_TYPE_3GPP),
	QMI_INIT(apn_disabled_flag, false),
};

static int
uqmi_wds_profile_type_parse(const char *type_string, QmiWdsProfileType *type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(profile_types); i++) {
		if (strcasecmp(profile_types[i].profile_name, type_string) != 0)
			continue;

		*type = profile_types[i].profile;
		return 0;
	}

	return -1;
}

static int
uqmi_wds_profile_identifier_parse(char *arg, struct uqmi_wds_profile_identifier *profile)
{
	char *s;
	char *p_type;
	int id;

	s = strchr(arg, ',');
	if (!s)
		return -1;
	*s = 0;
	s++;

	id = strtoul(s, &s, 0);
	if (s && *s)
		return -1;

	p_type = strtok(arg, ",");

	if (uqmi_wds_profile_type_parse(p_type, &profile->type))
		return -1;

	profile->index = id;

	return 0;
}

#define cmd_wds_set_apn_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_apn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, apn, arg);
	qmi_set_ptr(&wds_mp_req, apn_name, arg);
	qmi_set_ptr(&wds_cp_req, apn_name, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_auth_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_auth_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(auth_modes); i++) {
		if (strcasecmp(auth_modes[i].auth_name, arg) != 0)
			continue;

		qmi_set(&wds_sn_req, authentication_preference, auth_modes[i].auth);
		qmi_set(&wds_mp_req, authentication, auth_modes[i].auth);
		qmi_set(&wds_cp_req, authentication, auth_modes[i].auth);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid auth mode (valid: pap, chap, both, none)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_set_username_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_username_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, username, arg);
	qmi_set_ptr(&wds_mp_req, username, arg);
	qmi_set_ptr(&wds_cp_req, username, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_password_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_password_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_ptr(&wds_sn_req, password, arg);
	qmi_set_ptr(&wds_mp_req, password, arg);
	qmi_set_ptr(&wds_cp_req, password, arg);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_autoconnect_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_autoconnect_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set(&wds_sn_req, enable_autoconnect, true);
	qmi_set(&wds_stn_req, disable_autoconnect, true);
	return QMI_CMD_DONE;
}

#define cmd_wds_set_ip_family_pref_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_ip_family_pref_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(ipfam_modes); i++) {
		if (strcasecmp(ipfam_modes[i].ipfam_name, arg) != 0)
			continue;

		qmi_set(&wds_sn_req, ip_family_preference, ipfam_modes[i].mode);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid value (valid: ipv4, ipv6, unspecified)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_set_pdp_type_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_pdp_type_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pdp_types); i++) {
		if (strcasecmp(pdp_types[i].pdp_name, arg) != 0)
			continue;

		qmi_set(&wds_mp_req, pdp_type, pdp_types[i].type);
		qmi_set(&wds_cp_req, pdp_type, pdp_types[i].type);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid value (valid: ipv4, ipv6, ipv4v6)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_no_roaming_cb no_cb
static enum qmi_cmd_result
cmd_wds_no_roaming_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	if (strcmp(arg, "true") == 0) {
		qmi_set(&wds_mp_req, roaming_disallowed_flag, true);
		qmi_set(&wds_cp_req, roaming_disallowed_flag, true);
	} else if (strcmp(arg, "false") == 0) {
		qmi_set(&wds_mp_req, roaming_disallowed_flag, false);
		qmi_set(&wds_cp_req, roaming_disallowed_flag, false);
	} else {
		uqmi_add_error("Invalid value (true or false)");
		return QMI_CMD_EXIT;
	}
	return QMI_CMD_DONE;
}

#define cmd_wds_set_profile_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	uint32_t idx = strtoul(arg, NULL, 10);

	qmi_set(&wds_sn_req, profile_index_3gpp, idx);
	return QMI_CMD_DONE;
}

static void
cmd_wds_start_network_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_start_network_response res;

	qmi_parse_wds_start_network_response(msg, &res);
	if (res.set.packet_data_handle)
		blobmsg_add_u32(&status, NULL, res.data.packet_data_handle);
}

static enum qmi_cmd_result
cmd_wds_start_network_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_start_network_request(msg, &wds_sn_req);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_stop_network_cb no_cb
static enum qmi_cmd_result
cmd_wds_stop_network_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	uint32_t pdh = strtoul(arg, NULL, 0);

	qmi_set(&wds_stn_req, packet_data_handle, pdh);
	qmi_set_wds_stop_network_request(msg, &wds_stn_req);
	return QMI_CMD_REQUEST;
}

static void
cmd_wds_modify_profile_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_modify_profile_response res;
	qmi_parse_wds_modify_profile_response(msg, &res);
}

static enum qmi_cmd_result
cmd_wds_modify_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct uqmi_wds_profile_identifier profile;

	if (uqmi_wds_profile_identifier_parse(arg, &profile) < 0) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}

	qmi_set_ptr(&wds_mp_req, profile_identifier.profile_type, profile.type);
	qmi_set_ptr(&wds_mp_req, profile_identifier.profile_index, profile.index);
	qmi_set_wds_modify_profile_request(msg, &wds_mp_req);

	return QMI_CMD_REQUEST;
}

static void
cmd_wds_get_profile_list_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_get_profile_list_response res;
	void *p, *t, *root;
	int i;

	qmi_parse_wds_get_profile_list_response(msg, &res);

	root = blobmsg_open_table(&status, NULL);
	p = blobmsg_open_array(&status, "profiles");
	for (i = 0; i < res.data.profile_list_n; i++) {
		t = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "index", res.data.profile_list[i].profile_index);
		blobmsg_add_string(&status, "name", res.data.profile_list[i].profile_name);
		blobmsg_close_table(&status, t);
	}
	blobmsg_close_array(&status, p);
	blobmsg_close_table(&status, root);
}

static enum qmi_cmd_result
cmd_wds_get_profile_list_prepare(struct qmi_dev *qmi, struct qmi_request *req,
				 struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_get_profile_list_request pl_req = {};
	QmiWdsProfileType profile_type;

	if (uqmi_wds_profile_type_parse(arg, &profile_type) < 0) {
		uqmi_add_error("Invalid type (valid: 3gpp or 3gpp2)");
		return QMI_CMD_EXIT;
	}

	qmi_set(&pl_req, profile_type, profile_type);

	qmi_set_wds_get_profile_list_request(msg, &pl_req);
	return QMI_CMD_REQUEST;
}

static void
cmd_wds_create_profile_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_create_profile_response res;
	void *p;

	qmi_parse_wds_create_profile_response(msg, &res);

	if (res.set.profile_identifier) {
		p = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "created-profile", res.data.profile_identifier.profile_index);
		blobmsg_close_table(&status, p);
	}
}

static enum qmi_cmd_result
cmd_wds_create_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(profile_types); i++) {
		if (strcasecmp(profile_types[i].profile_name, arg) != 0)
			continue;

		qmi_set_ptr(&wds_cp_req, profile_type, profile_types[i].profile);

		qmi_set_wds_create_profile_request(msg, &wds_cp_req);
		return QMI_CMD_REQUEST;
	}

	uqmi_add_error("Invalid value (valid: 3gpp or 3gpp2)");
	return QMI_CMD_EXIT;
}

static void
cmd_wds_get_packet_service_status_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_get_packet_service_status_response res;
	const char *data_status[] = {
		[QMI_WDS_CONNECTION_STATUS_UNKNOWN] = "unknown",
		[QMI_WDS_CONNECTION_STATUS_DISCONNECTED] = "disconnected",
		[QMI_WDS_CONNECTION_STATUS_CONNECTED] = "connected",
		[QMI_WDS_CONNECTION_STATUS_SUSPENDED] = "suspended",
		[QMI_WDS_CONNECTION_STATUS_AUTHENTICATING] = "authenticating",
	};
	int s = 0;

	qmi_parse_wds_get_packet_service_status_response(msg, &res);
	if (res.set.connection_status &&
	    res.data.connection_status < ARRAY_SIZE(data_status))
		s = res.data.connection_status;

	blobmsg_add_string(&status, NULL, data_status[s]);
}

static enum qmi_cmd_result
cmd_wds_get_packet_service_status_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_get_packet_service_status_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_set_autoconnect_settings_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_autoconnect_settings_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_set_autoconnect_settings_request ac_req;
	const char *modes[] = {
		[QMI_WDS_AUTOCONNECT_SETTING_DISABLED] = "disabled",
		[QMI_WDS_AUTOCONNECT_SETTING_ENABLED] = "enabled",
		[QMI_WDS_AUTOCONNECT_SETTING_PAUSED] = "paused",
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (strcasecmp(modes[i], arg) != 0)
			continue;

		qmi_set(&ac_req, status, i);
		qmi_set_wds_set_autoconnect_settings_request(msg, &ac_req);
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid value (valid: disabled, enabled, paused)");
	return QMI_CMD_EXIT;
}

#define cmd_wds_reset_cb no_cb
static enum qmi_cmd_result
cmd_wds_reset_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_wds_reset_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_set_ip_family_cb no_cb
static enum qmi_cmd_result
cmd_wds_set_ip_family_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_set_ip_family_request ipf_req;

	int i;
	for (i = 0; i < ARRAY_SIZE(ipfam_modes); i++) {
		if (strcasecmp(ipfam_modes[i].ipfam_name, arg) != 0)
			continue;

		qmi_set(&ipf_req, preference, ipfam_modes[i].mode);
		qmi_set_wds_set_ip_family_request(msg, &ipf_req);
		return QMI_CMD_REQUEST;
	}

	uqmi_add_error("Invalid value (valid: ipv4, ipv6, unspecified)");
	return QMI_CMD_EXIT;
}

static struct {
	uint32_t type;
	uint32_t iface;
} wds_endpoint_info;

#define cmd_wds_bind_mux_cb no_cb

static enum qmi_cmd_result
cmd_wds_bind_mux_prepare(struct qmi_dev *qmi, struct qmi_request *req,
			 struct qmi_msg *msg, char *arg)
{
	uint32_t mux_num = strtoul(arg, NULL, 10);
	struct qmi_wds_bind_mux_data_port_request wds_mux_req = {
		QMI_INIT_SEQUENCE(
				endpoint_info,
				.endpoint_type = wds_endpoint_info.type,
				.interface_number = wds_endpoint_info.iface,
		),
		QMI_INIT(mux_id, mux_num),
		QMI_INIT(client_type, QMI_WDS_CLIENT_TYPE_TETHERED),
	};

	qmi_set_wds_bind_mux_data_port_request(msg, &wds_mux_req);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_ep_iface_cb no_cb

static enum qmi_cmd_result
cmd_wds_ep_iface_prepare(struct qmi_dev *qmi, struct qmi_request *req,
			 struct qmi_msg *msg, char *arg)
{
	uint32_t iface_num = strtoul(arg, NULL, 10);

	wds_endpoint_info.iface = iface_num;
	return QMI_CMD_DONE;
}

#define cmd_wds_ep_type_cb no_cb

static enum qmi_cmd_result
cmd_wds_ep_type_prepare(struct qmi_dev *qmi, struct qmi_request *req,
			struct qmi_msg *msg, char *arg)
{
	if (strcmp(arg, "hsusb") == 0) {
		wds_endpoint_info.type = QMI_DATA_ENDPOINT_TYPE_HSUSB;
	} else if (strcmp(arg, "pcie") == 0) {
		wds_endpoint_info.type = QMI_DATA_ENDPOINT_TYPE_PCIE;
	} else {
		uqmi_add_error("Invalid value (valid: hsusb, pcie)");
		return QMI_CMD_EXIT;
	}

	return QMI_CMD_DONE;
}

static void wds_to_ipv4(const char *name, const uint32_t addr)
{
	struct in_addr ip_addr;
	char buf[INET_ADDRSTRLEN];

	ip_addr.s_addr = htonl(addr);
	blobmsg_add_string(&status, name, inet_ntop(AF_INET, &ip_addr, buf, sizeof(buf)));
}

static void wds_to_ipv6(const char *name, const uint16_t *addr)
{
	char buf[INET6_ADDRSTRLEN];
	uint16_t ip_addr[8];
	int i;

	for (i = 0; i < ARRAY_SIZE(ip_addr); i++)
		ip_addr[i] = htons(addr[i]);

	blobmsg_add_string(&status, name, inet_ntop(AF_INET6, &ip_addr, buf, sizeof(buf)));
}

static enum qmi_cmd_result
cmd_wds_get_profile_settings_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct uqmi_wds_profile_identifier profile;

	if (uqmi_wds_profile_identifier_parse(arg, &profile) < 0) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}

	struct qmi_wds_get_profile_settings_request p_num = {
			QMI_INIT_SEQUENCE(profile_id,
				.profile_type = profile.type,
				.profile_index = profile.index,
			)
		};

	qmi_set_wds_get_profile_settings_request(msg, &p_num);
	return QMI_CMD_REQUEST;
}

static void
cmd_wds_get_profile_settings_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_get_profile_settings_response res;

	void *p;

	qmi_parse_wds_get_profile_settings_response(msg, &res);

	p = blobmsg_open_table(&status, NULL);

	blobmsg_add_string(&status, "apn", res.data.apn_name);
	if (res.set.pdp_type && (int) res.data.pdp_type < ARRAY_SIZE(pdp_types))
		blobmsg_add_string(&status, "pdp-type", pdp_types[res.data.pdp_type].pdp_name);
	blobmsg_add_string(&status, "username", res.data.username);
	blobmsg_add_string(&status, "password", res.data.password);
        if (res.set.authentication && (int) res.data.authentication < ARRAY_SIZE(auth_modes))
                blobmsg_add_string(&status, "auth", auth_modes[res.data.authentication].auth_name);
	blobmsg_add_u8(&status, "no-roaming", res.data.roaming_disallowed_flag);
	blobmsg_add_u8(&status, "apn-disabled", res.data.apn_disabled_flag);
	blobmsg_close_table(&status, p);
}

static void
cmd_wds_get_current_settings_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	void *v4, *v6, *d, *t;
	struct qmi_wds_get_current_settings_response res;
	int i;

	qmi_parse_wds_get_current_settings_response(msg, &res);

	t = blobmsg_open_table(&status, NULL);

	if (res.set.pdp_type && (int) res.data.pdp_type < ARRAY_SIZE(pdp_types))
		blobmsg_add_string(&status, "pdp-type", pdp_types[res.data.pdp_type].pdp_name);

	if (res.set.ip_family) {
		for (i = 0; i < ARRAY_SIZE(ipfam_modes); i++) {
			if (ipfam_modes[i].mode != res.data.ip_family)
				continue;
			blobmsg_add_string(&status, "ip-family", ipfam_modes[i].ipfam_name);
			break;
		}
	}

	if (res.set.mtu)
		blobmsg_add_u32(&status, "mtu", res.data.mtu);

	/* IPV4 */
	v4 = blobmsg_open_table(&status, "ipv4");

	if (res.set.ipv4_address)
		wds_to_ipv4("ip", res.data.ipv4_address);
	if (res.set.primary_ipv4_dns_address)
		wds_to_ipv4("dns1", res.data.primary_ipv4_dns_address);
	if (res.set.secondary_ipv4_dns_address)
		wds_to_ipv4("dns2", res.data.secondary_ipv4_dns_address);
	if (res.set.ipv4_gateway_address)
		wds_to_ipv4("gateway", res.data.ipv4_gateway_address);
	if (res.set.ipv4_gateway_subnet_mask)
		wds_to_ipv4("subnet", res.data.ipv4_gateway_subnet_mask);
	blobmsg_close_table(&status, v4);

	/* IPV6 */
	v6 = blobmsg_open_table(&status, "ipv6");

	if (res.set.ipv6_address) {
		wds_to_ipv6("ip", res.data.ipv6_address.address);
		blobmsg_add_u32(&status, "ip-prefix-length", res.data.ipv6_address.prefix_length);
	}
	if (res.set.ipv6_gateway_address) {
		wds_to_ipv6("gateway", res.data.ipv6_gateway_address.address);
		blobmsg_add_u32(&status, "gw-prefix-length", res.data.ipv6_gateway_address.prefix_length);
	}
	if (res.set.ipv6_primary_dns_address)
		wds_to_ipv6("dns1", res.data.ipv6_primary_dns_address);
	if (res.set.ipv6_secondary_dns_address)
		wds_to_ipv6("dns2", res.data.ipv6_secondary_dns_address);

	blobmsg_close_table(&status, v6);

	d = blobmsg_open_table(&status, "domain-names");
	for (i = 0; i < res.data.domain_name_list_n; i++) {
		blobmsg_add_string(&status, NULL, res.data.domain_name_list[i]);
	}
	blobmsg_close_table(&status, d);

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_wds_get_current_settings_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_get_current_settings_request gcs_req;
	memset(&gcs_req, '\0', sizeof(struct qmi_wds_get_current_settings_request));
	qmi_set(&gcs_req, requested_settings,
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_PDP_TYPE |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_DNS_ADDRESS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_GRANTED_QOS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_IP_ADDRESS |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_GATEWAY_INFO |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_MTU |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_DOMAIN_NAME_LIST |
		QMI_WDS_GET_CURRENT_SETTINGS_REQUESTED_SETTINGS_IP_FAMILY);
	qmi_set_wds_get_current_settings_request(msg, &gcs_req);
	return QMI_CMD_REQUEST;
}

static enum qmi_cmd_result
cmd_wds_get_default_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(profile_types); i++) {
		if (strcasecmp(profile_types[i].profile_name, arg) != 0)
			continue;

		struct qmi_wds_get_default_profile_number_request type_family = {
			QMI_INIT_SEQUENCE(profile_type,
				.type = profile_types[i].profile,
				.family = QMI_WDS_PROFILE_FAMILY_TETHERED,
			)
		};

		qmi_set_wds_get_default_profile_number_request(msg, &type_family);
		return QMI_CMD_REQUEST;
	}

	uqmi_add_error("Invalid value (valid: 3gpp or 3gpp2)");
	return QMI_CMD_EXIT;
}

static void
cmd_wds_get_default_profile_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wds_get_default_profile_number_response res;
	void *p;
	qmi_parse_wds_get_default_profile_number_response(msg, &res);

	p = blobmsg_open_table(&status, NULL);

	blobmsg_add_u32(&status, "default-profile", res.data.index);

	blobmsg_close_table(&status, p);
}

#define cmd_wds_set_default_profile_cb no_cb

static enum qmi_cmd_result
cmd_wds_set_default_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req,
				    struct qmi_msg *msg, char *arg)
{
	struct uqmi_wds_profile_identifier profile;

	if (uqmi_wds_profile_identifier_parse(arg, &profile) < 0) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}

	struct qmi_wds_set_default_profile_number_request set_default_profile = {
		QMI_INIT_SEQUENCE(profile_identifier,
			.family = QMI_WDS_PROFILE_FAMILY_TETHERED,
			.type = profile.type,
			.index = profile.index,
		)
	};

	qmi_set_wds_set_default_profile_number_request(msg, &set_default_profile);
	return QMI_CMD_REQUEST;
}

#define cmd_wds_delete_profile_cb no_cb

static enum qmi_cmd_result
cmd_wds_delete_profile_prepare(struct qmi_dev *qmi, struct qmi_request *req,
			       struct qmi_msg *msg, char *arg)
{
	struct qmi_wds_delete_profile_request delete_req = {
		QMI_INIT_SEQUENCE(profile_identifier,
			.profile_type = QMI_WDS_PROFILE_TYPE_3GPP,
			.profile_index = 1,
		)
	};
	struct uqmi_wds_profile_identifier profile;

	if (uqmi_wds_profile_identifier_parse(arg, &profile) < 0) {
		fprintf(stderr, "Invalid argument\n");
		return QMI_CMD_EXIT;
	}

	qmi_set_ptr(&delete_req, profile_identifier.profile_type, profile.type);
	qmi_set_ptr(&delete_req, profile_identifier.profile_index, profile.index);

	qmi_set_wds_delete_profile_request(msg, &delete_req);

	return QMI_CMD_REQUEST;
}

#define cmd_wds_set_lte_attach_pdn_cb no_cb

static enum qmi_cmd_result
cmd_wds_set_lte_attach_pdn_prepare(struct qmi_dev *qmi, struct qmi_request *req,
				   struct qmi_msg *msg, char *arg)
{
	uint16_t list[8] = {0};

	char *s = arg;
	int i = 0;

	while (s) {
		if (i >= 8) {
			fprintf(stderr, "Only 8 attach PDN supported\n");
			return QMI_CMD_EXIT;
		}

		list[i] = strtoul(s, &s, 10);
		i++;
		if (*s == ',') {
			s++;
		} else {
			fprintf(stderr, "Invalid argument\n");
			return QMI_CMD_EXIT;
		}
	}


	struct qmi_wds_set_lte_attach_pdn_list_request lte_attach_pdn = {
		QMI_INIT_ARRAY(list, list, i),
		QMI_INIT(action, QMI_WDS_ATTACH_PDN_LIST_ACTION_DETACH_OR_PDN_DISCONNECT)
	};

	qmi_set_wds_set_lte_attach_pdn_list_request(msg, &lte_attach_pdn);
	return QMI_CMD_REQUEST;
}
