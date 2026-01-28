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

#include "qmi-message.h"

static const struct {
	const char *name;
	QmiWdaLinkLayerProtocol val;
} link_modes[] = {
	{ "802.3", QMI_WDA_LINK_LAYER_PROTOCOL_802_3 },
	{ "raw-ip", QMI_WDA_LINK_LAYER_PROTOCOL_RAW_IP },
};

static const struct {
	const char *name;
	QmiWdaDataAggregationProtocol aggreg;
} aggregation_protocols[] = {
	{ "tlp", QMI_WDA_DATA_AGGREGATION_PROTOCOL_TLP },
	{ "qc-cm", QMI_WDA_DATA_AGGREGATION_PROTOCOL_QC_NCM },
	{ "mbim", QMI_WDA_DATA_AGGREGATION_PROTOCOL_MBIM },
	{ "rndis", QMI_WDA_DATA_AGGREGATION_PROTOCOL_RNDIS },
	{ "qmap", QMI_WDA_DATA_AGGREGATION_PROTOCOL_QMAP },
	{ "qmapv5", QMI_WDA_DATA_AGGREGATION_PROTOCOL_QMAPV5 },
};

static struct {
	uint32_t max_size_dl;
	uint32_t max_datagrams_dl;
	QmiWdaDataAggregationProtocol aggregation_protocol_dl;
	uint32_t max_size_ul;
	uint32_t max_datagrams_ul;
	QmiWdaDataAggregationProtocol aggregation_protocol_ul;
	int8_t flow_control;
} wda_aggregation_info = {
	.max_size_dl = 0,
	.max_datagrams_dl = 0,
	.aggregation_protocol_dl = QMI_WDA_DATA_AGGREGATION_PROTOCOL_DISABLED,
	.max_size_ul = 0,
	.max_datagrams_ul = 0,
	.aggregation_protocol_ul = QMI_WDA_DATA_AGGREGATION_PROTOCOL_DISABLED,
	.flow_control = -1,
};

#define cmd_wda_set_data_format_cb no_cb

static void
cmd_wda_set_data_format_send(struct qmi_msg *msg, QmiWdaLinkLayerProtocol link_layer_proto)
{
	struct qmi_wda_set_data_format_request data_req = {
		QMI_INIT(link_layer_protocol, link_layer_proto),
		QMI_INIT(uplink_data_aggregation_protocol, wda_aggregation_info.aggregation_protocol_ul),
		QMI_INIT(uplink_data_aggregation_max_datagrams, wda_aggregation_info.max_datagrams_ul),
		QMI_INIT(uplink_data_aggregation_max_size, wda_aggregation_info.max_size_ul),
		QMI_INIT(downlink_data_aggregation_protocol, wda_aggregation_info.aggregation_protocol_dl),
		QMI_INIT(downlink_data_aggregation_max_datagrams, wda_aggregation_info.max_datagrams_dl),
		QMI_INIT(downlink_data_aggregation_max_size, wda_aggregation_info.max_size_dl),
		QMI_INIT(downlink_minimum_padding, 0),
	};

	if (wda_aggregation_info.flow_control >= 0)
		qmi_set(&data_req, flow_control, wda_aggregation_info.flow_control);

	qmi_set_wda_set_data_format_request(msg, &data_req);
}

static enum qmi_cmd_result
cmd_wda_set_data_format_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiWdaLinkLayerProtocol link_layer_proto;
	int i;

	for (i = 0; i < ARRAY_SIZE(link_modes); i++) {
		if (strcasecmp(link_modes[i].name, arg) != 0)
			continue;

		link_layer_proto = link_modes[i].val;
	}

	if (link_layer_proto == QMI_WDA_LINK_LAYER_PROTOCOL_UNKNOWN) {
		uqmi_add_error("Invalid auth mode (valid: 802.3, raw-ip)");
		return QMI_CMD_EXIT;
	}

	cmd_wda_set_data_format_send(msg, link_layer_proto);

	return QMI_CMD_REQUEST;
}

#define cmd_wda_downlink_data_aggregation_protocol_cb no_cb

static enum qmi_cmd_result cmd_wda_downlink_data_aggregation_protocol_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(aggregation_protocols); i++) {
		if (strcasecmp(aggregation_protocols[i].name, arg))
			continue;

		wda_aggregation_info.aggregation_protocol_dl = aggregation_protocols[i].aggreg;
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid protocol (valid: tlp, qc-cm, mbim, rndis, qmap, qmapv5");
	return QMI_CMD_EXIT;
}

#define cmd_wda_uplink_data_aggregation_protocol_cb no_cb

static enum qmi_cmd_result cmd_wda_uplink_data_aggregation_protocol_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(aggregation_protocols); i++) {
		if (strcasecmp(aggregation_protocols[i].name, arg))
			continue;

		wda_aggregation_info.aggregation_protocol_ul = aggregation_protocols[i].aggreg;
		return QMI_CMD_DONE;
	}

	uqmi_add_error("Invalid protocol (valid: tlp, qc-cm, mbim, rndis, qmap, qmapv5");
	return QMI_CMD_EXIT;
}

#define cmd_wda_downlink_data_aggregation_max_datagrams_cb no_cb

static enum qmi_cmd_result cmd_wda_downlink_data_aggregation_max_datagrams_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	uint32_t max_datagrams = strtoul(arg, NULL, 10);

	wda_aggregation_info.max_datagrams_dl = max_datagrams;
	return QMI_CMD_DONE;
}

#define cmd_wda_downlink_data_aggregation_max_size_cb no_cb

static enum qmi_cmd_result cmd_wda_downlink_data_aggregation_max_size_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	uint32_t max_size = strtoul(arg, NULL, 10);

	wda_aggregation_info.max_size_dl = max_size;
	return QMI_CMD_DONE;
}

#define cmd_wda_uplink_data_aggregation_max_datagrams_cb no_cb

static enum qmi_cmd_result cmd_wda_uplink_data_aggregation_max_datagrams_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	uint32_t max_datagrams = strtoul(arg, NULL, 10);

	wda_aggregation_info.max_datagrams_ul = max_datagrams;
	return QMI_CMD_DONE;
}

#define cmd_wda_uplink_data_aggregation_max_size_cb no_cb

static enum qmi_cmd_result cmd_wda_uplink_data_aggregation_max_size_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	uint32_t max_size = strtoul(arg, NULL, 10);

	wda_aggregation_info.max_size_ul = max_size;
	return QMI_CMD_DONE;
}

#define cmd_wda_flow_control_cb no_cb

static enum qmi_cmd_result cmd_wda_flow_control_prepare(
	struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg,
	char *arg)
{
	uint32_t val = strtoul(arg, NULL, 10);

	if (val != 0 && val != 1) {
		uqmi_add_error("Invalid value (valid: 0, 1)");
		return QMI_CMD_EXIT;
	}

	wda_aggregation_info.flow_control = !!val;

	return QMI_CMD_DONE;
}

static const char *
wda_link_layer_protocol_to_string(QmiWdaLinkLayerProtocol proto)
{
	for (int i = 0; i < ARRAY_SIZE(link_modes); i++) {
		if (link_modes[i].val == proto)
			return link_modes[i].name;
	}
	return "unknown";
}

static const char *
wda_data_aggregation_protocol_to_string(QmiWdaDataAggregationProtocol proto)
{
	for (int i = 0; i < ARRAY_SIZE(aggregation_protocols); i++) {
		if (aggregation_protocols[i].aggreg == proto)
			return aggregation_protocols[i].name;
	}
	return "unknown";
}

static void
cmd_wda_get_data_format_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wda_get_data_format_response res;
	void *root;

	qmi_parse_wda_get_data_format_response(msg, &res);
	root = blobmsg_open_table(&status, NULL);
	blobmsg_add_u8(&status, "qos-format", res.data.qos_format);
	blobmsg_add_string(&status, "link-layer-protocol",
			   wda_link_layer_protocol_to_string(res.data.link_layer_protocol));
	blobmsg_add_string(&status, "data-aggregation-protocol",
			   wda_data_aggregation_protocol_to_string(res.data.uplink_data_aggregation_protocol));
	blobmsg_add_u32(&status, "uplink-data-aggregation-max-datagrams",
			res.data.uplink_data_aggregation_max_datagrams);
	blobmsg_add_u32(&status, "uplink-data-aggregation-max-size",
			res.data.uplink_data_aggregation_max_size);
	blobmsg_add_string(&status, "downlink-data-aggregation-protocol",
			   wda_data_aggregation_protocol_to_string(res.data.downlink_data_aggregation_protocol));
	blobmsg_add_u32(&status, "downlink-data-aggregation-max-datagrams",
			res.data.downlink_data_aggregation_max_datagrams);
	blobmsg_add_u32(&status, "downlink-data-aggregation-max-size",
			res.data.downlink_data_aggregation_max_size);
	blobmsg_add_u32(&status, "download-minimum-padding",
			res.data.download_minimum_padding);
	blobmsg_add_u8(&status, "flow-control", res.data.flow_control);
	blobmsg_close_table(&status, root);
}

static enum qmi_cmd_result
cmd_wda_get_data_format_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_wda_get_data_format_request data_req = {};
	qmi_set_wda_get_data_format_request(msg, &data_req);
	return QMI_CMD_REQUEST;
}
