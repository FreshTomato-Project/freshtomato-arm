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

#ifndef __UQMID_QMI_DEV_H
#define __UQMID_QMI_DEV_H

#include <stdint.h>
#include <libubox/list.h>

#include "uqmid.h"

struct qmi_dev;
struct qmi_request;

enum {
	SERVICE_IDLE,
	SERVICE_WAIT_CID,
	SERVICE_READY,
};

struct qmi_service {
	int service; /*! service id */
	uint16_t client_id; /*! client id, unique per service */
	uint16_t tid; /*! the current transaction id */

	uint16_t major;
	uint16_t minor;

	int state;

	struct qmi_dev *qmi;
	struct list_head list; /*! entry on qmi_dev */

	/* contains all pending requests */
	struct list_head reqs;

	/* contains indication registers
	 * a sorted llist by qmi msg id
	 */
	struct list_head indications;
};

typedef void (*indication_cb)(struct qmi_service *service, struct qmi_msg *msg, void *data);
struct qmi_indication {
	/* a sorted linked list by qmi_ind in indications */
	struct list_head list;
	/* qmi_ind the qmi message id.
	 * Multiple callbacks can receive the same indication
	 */
	uint16_t qmi_ind;
	indication_cb cb;
	void *cb_data;
};

struct qmi_service *uqmi_service_find(struct qmi_dev *qmi, int service_id);
struct qmi_service *uqmi_service_find_or_init(struct qmi_dev *qmi, int service_id);
int uqmi_service_open(struct qmi_dev *qmi);
void uqmi_service_close(struct qmi_service *service);
int uqmi_service_send_msg(struct qmi_service *service, struct qmi_request *req);
int uqmi_service_send_msg2(struct qmi_dev *qmi, struct qmi_request *req, int service_id);
int uqmi_service_send_simple(struct qmi_service *service,
			 int(*encode)(struct qmi_msg *msg),
			 request_cb cb, void *cb_data);

int uqmi_service_get_next_tid(struct qmi_service *service);
struct qmi_service *uqmi_service_create(struct qmi_dev *qmi, int service_id);

/* callback for the ctrl when allocating the client id */
void uqmi_service_get_client_id_cb(struct qmi_service *service, uint16_t client_id);
void uqmi_service_close_cb(struct qmi_service *service);

/* indications */

int uqmi_service_register_indication(struct qmi_service *service, uint16_t indication, indication_cb cb, void *cb_data);
int uqmi_service_remove_indication(struct qmi_service *service, uint16_t indication, indication_cb cb, void *cb_data);
int uqmi_service_handle_indication(struct qmi_service *service, struct qmi_msg *msg);

#endif /* __UQMID_QMI_DEV_H */
