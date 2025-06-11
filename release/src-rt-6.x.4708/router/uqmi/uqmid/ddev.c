
#include <stddef.h>
#include <fcntl.h>
#include <talloc.h>

#include <libubox/list.h>
#include <libubox/utils.h>

#include "qmi-enums.h"
#include "qmi-enums-private.h"
#include "qmi-message.h"
#include "qmi-struct.h"

#include "ctrl.h"
#include "uqmid.h"
#include "logging.h"
#include "services.h"
#include "modem.h"
#include "gsmtap_util.h"

/* FIXME: decide dump_packet */
#define dump_packet(str, buf, len)

static void
__qmi_request_complete(struct qmi_service *service, struct qmi_request *req, struct qmi_msg *msg)
{
	void *tlv_buf;
	int tlv_len;

	if (!req->pending)
		return;

	req->pending = false;
	req->complete = true;
	list_del(&req->list);

	if (msg) {
		tlv_buf = qmi_msg_get_tlv_buf(msg, &tlv_len);
		req->ret = qmi_check_message_status(tlv_buf, tlv_len);
	} else {
		req->ret = QMI_ERROR_CANCELLED;
	}

	if (req->cb && msg)
		req->cb(service, req, msg);

	talloc_free(req);
	/* frees msg as well because of tree */
}

static void
qmi_process_msg(struct qmi_dev *qmi, struct qmi_msg *msg)
{
	struct qmi_service *service;
	struct qmi_request *req;
	uint16_t tid;
	bool resp, ind;

	if (msg->qmux.service == QMI_SERVICE_CTL) {
		modem_log(qmi->modem, LOGL_DEBUG, "Process message from srv %d msg %04x flag: %02x tid: %02x",
			  msg->qmux.service, le16_to_cpu(msg->ctl.message), msg->flags, msg->ctl.transaction);
		tid = msg->ctl.transaction;
		ind = msg->flags & QMI_CTL_FLAG_INDICATION;
		resp = msg->flags & QMI_CTL_FLAG_RESPONSE;
		if (!ind && !resp) {
			/* TODO: error_log("Invalid message received") */
			return;
		}
	} else {
		modem_log(qmi->modem, LOGL_DEBUG, "Process message from srv %d msg %04x flag: %02x tid: %04x",
			  msg->qmux.service, le16_to_cpu(msg->svc.message), msg->flags, msg->svc.transaction);
		tid = le16_to_cpu(msg->svc.transaction);
		ind = msg->flags & QMI_SERVICE_FLAG_INDICATION;
		resp = msg->flags & QMI_SERVICE_FLAG_RESPONSE;
		if (!ind && !resp) {
			/* TODO: error_log("Invalid message received") */
			return;
		}
	}

	service = uqmi_service_find(qmi, msg->qmux.service);
	if (!service) {
		/* error_log("Couldn't find a service for incoming message") */
		return;
	}

	/* Hopefully an indication *and* response isn't possible */
	if (ind) {
		uqmi_service_handle_indication(service, msg);
	}

	if (resp) {
		list_for_each_entry(req, &service->reqs, list) {
			if (req->tid != tid)
				continue;

			__qmi_request_complete(service, req, msg);
			return;
		}
	}

	/* error_log("Couldn't find a tid for incoming message") */
}

static void qmi_notify_read(struct ustream *us, int bytes)
{
	struct qmi_dev *qmi = container_of(us, struct qmi_dev, sf.stream);
	struct qmi_msg *msg;
	char *buf;
	int len, msg_len;


	while (1) {
		buf = ustream_get_read_buf(us, &len);
		if (!buf || !len)
			return;

		/* FIXME: check partial reads */
		/* FIXME: implement mbim */

		dump_packet("Received packet", buf, len);
		if (len < offsetof(struct qmi_msg, flags))
			return;
		msg = (struct qmi_msg *) buf;
		msg_len = le16_to_cpu(msg->qmux.len) + 1;

		if (len < msg_len)
			return;

		gsmtap_send(qmi->modem, msg, msg_len);
		qmi_process_msg(qmi, msg);
		ustream_consume(us, msg_len);
	}
}

static void qmi_notify_state(struct ustream *us)
{
	struct qmi_dev *qmi = container_of(us, struct qmi_dev, sf.stream);

	if (us->eof || us->write_error) {
		modem_log(qmi->modem, LOGL_ERROR, "Modem connection died! Closing modem.");
	} else {
		modem_log(qmi->modem, LOGL_ERROR, "Unknown modem fd state change eof: %d write_error: %d. Closing modem anyways",
			  us->eof, us->write_error);
	}

	/* errors! */
	if (qmi->state != QMI_STOPPING)
		qmi->state = QMI_ERROR;

	if (qmi->error_cb)
		qmi->error_cb(qmi, qmi->error_cb_data);
}

struct qmi_dev *qmi_device_open(struct modem *modem, const char *path)
{
	struct qmi_dev *qmi;
	struct ustream *us;
	int fd;

	/* assert(qmi) */

	fd = open(path, O_RDWR | O_EXCL | O_NONBLOCK | O_NOCTTY);
	if (fd < 0)
		return NULL;

	qmi = talloc_zero(modem, struct qmi_dev);
	us = &qmi->sf.stream;

	us->notify_state = qmi_notify_state;
	us->notify_read = qmi_notify_read;
	ustream_fd_init(&qmi->sf, fd);
	INIT_LIST_HEAD(&qmi->services);
	qmi->modem = modem;

	uqmi_ctrl_generate(qmi);

	return qmi;
}

/* timer callback to give service the time to shut down */
static void qmi_device_close_cb(struct uloop_timeout *timeout)
{
	struct qmi_service *service, *tmp;
	struct qmi_dev *qmi = container_of(timeout, struct qmi_dev, shutdown);

	modem_log(qmi->modem, LOGL_INFO, "Closing qmi device");
	uqmi_service_close(qmi->ctrl);
	list_for_each_entry_safe(service, tmp, &qmi->services, list) {
		list_del(&service->list);
		talloc_free(service);
	}
	qmi->ctrl = NULL;

	ustream_free(&qmi->sf.stream);
	close(qmi->sf.fd.fd);

	if (qmi->closing_cb)
		qmi->closing_cb(qmi, qmi->closing_cb_data);
}

/* called by the service when the QMI modem release the client id */
void qmi_device_service_closed(struct qmi_dev *qmi)
{
	if (qmi->state != QMI_STOPPING)
		return;
	/* only ctrl left, use schedule to decouple it from req and break a free(req) loop */
	if (qmi->services.next == qmi->services.prev && qmi->services.prev == &qmi->ctrl->list)
		uloop_timeout_set(&qmi->shutdown, 0);
}

void qmi_device_close(struct qmi_dev *qmi, int timeout_ms)
{
	struct qmi_service *service, *tmp;
	bool error = qmi->state == QMI_ERROR;

	if (qmi->state == QMI_STOPPING)
		return;

	qmi->state = QMI_STOPPING;
	if (!error) {
		list_for_each_entry_safe(service, tmp, &qmi->services, list) {
			/* CTL service is required to close the others. The pending request will be cleared in _cb */
			if (service->service == QMI_SERVICE_CTL)
				continue;

			uqmi_service_close(service);
		}
	}

	/* should we allow to close all services at once or should we close it slowly? one-by-one? */
	if (timeout_ms <= 0 || error) {
		qmi_device_close_cb(&qmi->shutdown);
	} else {
		qmi->shutdown.cb = qmi_device_close_cb;
		uloop_timeout_set(&qmi->shutdown, timeout_ms);
	}
}
