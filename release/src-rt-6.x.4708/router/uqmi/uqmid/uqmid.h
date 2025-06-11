/*
 * uqmid -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
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

#ifndef __UQMID_H
#define __UQMID_H

#include <libubox/ustream.h>

enum {
	L_CRIT,
	L_WARNING,
	L_NOTICE,
	L_INFO,
	L_DEBUG
};

struct qmi_dev;
struct qmi_service;
struct qmi_request;
struct qmi_msg;
struct modem;

typedef void (*request_cb)(struct qmi_service *service, struct qmi_request *req, struct qmi_msg *msg);

struct qmi_request {
	struct list_head list; /*! entry on service->reqs */
	struct qmi_service *service; /*! pointer back */

	struct qmi_msg *msg;

	request_cb cb;
	void *cb_data;

	bool complete;
	bool pending;
	bool no_error_cb;
	uint16_t tid;
	int ret;
};

/* FIXME: describe state machine */
enum {
	QMI_RUNNING,
	QMI_ERROR,
	QMI_STOPPING,
};

struct qmi_dev {
	struct ustream_fd sf;

	struct list_head services;
	struct qmi_service *ctrl;
	struct modem *modem;

	bool is_mbim;

	struct {
		bool valid;
		uint8_t profile_id;
		uint8_t packet_handle;
		uint8_t ip_family;
	} wds;

	int state;
	struct uloop_timeout shutdown;
	void (*error_cb)(struct qmi_dev *dev, void *data);
	void *error_cb_data;
	void (*closing_cb)(struct qmi_dev *dev, void *data);
	void *closing_cb_data;
};

struct qmi_dev *qmi_device_open(struct modem *modem, const char *path);
void qmi_device_close(struct qmi_dev *qmi, int timeout_ms);
void qmi_device_service_closed(struct qmi_dev *qmi);

#endif /* __UQMID_H */
