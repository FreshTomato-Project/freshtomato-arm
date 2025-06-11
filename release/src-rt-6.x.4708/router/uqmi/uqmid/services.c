/* similar to dev.c but self container */

#include <stdint.h>

#include <talloc.h>
#include <errno.h>

#include "qmi-enums.h"
#include "qmi-message.h"

#include "ctrl.h"
#include "logging.h"
#include "modem.h"
#include "services.h"
#include "uqmid.h"

#include "gsmtap_util.h"

#ifdef DEBUG_PACKET
static void dump_packet(const char *prefix, void *ptr, int len)
{
	unsigned char *data = ptr;
	int i;

	fprintf(stderr, "%s:", prefix);
	for (i = 0; i < len; i++)
		fprintf(stderr, " %02x", data[i]);
	fprintf(stderr, "\n");
}
#else
static void dump_packet(const char *prefix, void *ptr, int len)
{
}
#endif

struct qmi_service *
uqmi_service_find(struct qmi_dev *qmi, int service_id)
{
	struct qmi_service *service;
	list_for_each_entry(service, &qmi->services, list) {
		if (service->service == service_id)
			return service;
	}

	return NULL;
}

struct qmi_service *
uqmi_service_create(struct qmi_dev *qmi, int service_id)
{
	struct qmi_service *service = talloc_zero(qmi, struct qmi_service);

	service->service = service_id;
	service->qmi = qmi;
	service->client_id = -1;

	list_add(&service->list, &qmi->services);
	INIT_LIST_HEAD(&service->indications);
	INIT_LIST_HEAD(&service->reqs);

	return service;
}

struct qmi_service *
uqmi_service_find_or_init(struct qmi_dev *qmi, int service_id)
{
	struct qmi_service *service = uqmi_service_find(qmi, service_id);

	if (service)
		return service;

	return uqmi_service_create(qmi, service_id);
}

int
uqmi_service_get_next_tid(struct qmi_service *service)
{
	service->tid++;
	/* CTL only has 8 bit tid */
	if (service->service == QMI_SERVICE_CTL && service->tid >= (1 << 8))
			service->tid = 1;
	else if (!service->tid)
			service->tid = 1;

	return service->tid;
}

static int
_service_send_request(struct qmi_service *service, struct qmi_request *req)
{
	struct qmi_msg *msg = req->msg;
	int len = qmi_complete_request_message(msg);
	uint16_t tid = uqmi_service_get_next_tid(service);

	if (req->service->service == QMI_SERVICE_CTL) {
		msg->ctl.transaction = tid;
	} else {
		/* FIXME: check if service state is ready for this */
		msg->svc.transaction = cpu_to_le16(tid);
		msg->qmux.client = service->client_id;
	}

	req->ret = -1;
	req->tid = tid;
	req->pending = true;

	/* FIXME: fix mbim support */

	if (service->service == QMI_SERVICE_CTL)
		modem_log(service->qmi->modem, LOGL_DEBUG, "Transmit message to srv %d msg %04x flag: %02x tid: %02x",
			  msg->qmux.service, le16_to_cpu(msg->ctl.message), msg->flags, le16_to_cpu(msg->ctl.transaction));
	else
		modem_log(service->qmi->modem, LOGL_DEBUG, "Transmit message to srv %d msg %04x flag: %02x tid: %04x",
			  msg->qmux.service, le16_to_cpu(msg->svc.message), msg->flags, le16_to_cpu(msg->svc.transaction));

	dump_packet("Send packet", msg, len);
	gsmtap_send(service->qmi->modem, msg, len);
	ustream_write(&service->qmi->sf.stream, (void *) msg, len, false);

	return 0;
}

/* send out all pending request */
static int
uqmi_service_send_request(struct qmi_service *service)
{
	struct qmi_request *req;
	list_for_each_entry(req, &service->reqs, list) {
		if (!req->pending && !req->complete)
			_service_send_request(service, req);
	}

	return 0;
}

void
uqmi_service_get_client_id_cb(struct qmi_service *service, uint16_t client_id)
{
	service->client_id = client_id;
	service->state = SERVICE_READY;

	service_log(service, LOGL_INFO, "Assigned client id %d. Service READY", client_id);
	uqmi_service_send_request(service);
}

/* get a client id via ctrl service */
static int
uqmi_service_get_client_id(struct qmi_service *service)
{
	if (service->service == QMI_SERVICE_CTL) {
		fprintf(stderr, "Foo!!!");
		return -EINVAL;
	}

	service_log(service, LOGL_INFO, "Request client id");

	service->state = SERVICE_WAIT_CID;
	uqmi_ctrl_request_clientid(service);

	return 0;
}

int
uqmi_service_send_simple(struct qmi_service *service,
			 int(*encoder)(struct qmi_msg *msg),
			 request_cb cb, void *cb_data)
{
	struct qmi_request *req = talloc_zero(service, struct qmi_request);
	struct qmi_msg *msg = talloc_zero_size(req, 1024);

	req->msg = msg;
	req->cb = cb;
	req->cb_data = cb_data;

	int ret = encoder(msg);
	if (ret) {
		cb(service, req, NULL);
		talloc_free(req);
		modem_log(service->qmi->modem, LOGL_ERROR, "Failed to encode in send_simple");
		return -1;
	}

	return uqmi_service_send_msg(service, req);
}

int
uqmi_service_send_msg(struct qmi_service *service, struct qmi_request *req)
{
	req->pending = false;
	req->complete = false;
	req->service = service;

	list_add(&req->list, &service->reqs);
	if (service->state == SERVICE_IDLE)
		return uqmi_service_get_client_id(service);
	else
		return uqmi_service_send_request(service);
}

int
uqmi_service_send_msg2(struct qmi_dev *qmi, struct qmi_request *req, int service_id)
{
	struct qmi_service *service = uqmi_service_find_or_init(qmi, service_id);

	if (!service) {
		/* FIXME: error_log("Couldn't find/create service for id %d", service_id) */
		return -EINVAL;
	}

	req->pending = false;
	req->complete = false;
	req->service = service;

	list_add(&req->list, &service->reqs);
	if (service->state == SERVICE_IDLE)
		return uqmi_service_get_client_id(service);
	else
		return uqmi_service_send_request(service);
}

/* called when the call id returns */
void uqmi_service_close_cb(struct qmi_service *service)
{
	struct qmi_dev *qmi = service->qmi;
	service_log(service, LOGL_INFO, "Released service.");

	list_del(&service->list);
	talloc_free(service);
	qmi_device_service_closed(qmi);
}

void uqmi_service_close(struct qmi_service *service)
{
	if (service->service == QMI_SERVICE_CTL)
		return;

	service_log(service, LOGL_INFO, "Closing service %d (cid %d)", service->service, service->client_id);

	/* FIXME: SERVICE_WAIT_CID might/will leak a CID */
	if (service->state != SERVICE_READY) {
		uqmi_service_close_cb(service);
		return;
	}

	/* Control service is special */
	uqmi_ctrl_release_clientid(service);
}

int uqmi_service_register_indication(struct qmi_service *service, uint16_t qmi_ind, indication_cb cb, void *cb_data)
{
	struct qmi_indication *indication;

	indication = talloc_zero(service, struct qmi_indication);
	if (!indication)
		return 1;

	indication->cb = cb;
	indication->cb_data = cb_data;
	indication->qmi_ind = qmi_ind;
	list_add(&indication->list, &service->indications);

	return 0;
}

int uqmi_service_remove_indication(struct qmi_service *service, uint16_t qmi_ind, indication_cb cb, void *cb_data)
{
	struct qmi_indication *indication, *tmp;

	list_for_each_entry_safe(indication, tmp, &service->indications, list) {
		if (qmi_ind != indication->qmi_ind)
			continue;

		if (indication->cb != cb)
			continue;

		if (indication->cb_data != cb_data)
			continue;

		list_del(&indication->list);
	}

	return 0;
}

int uqmi_service_handle_indication(struct qmi_service *service, struct qmi_msg *msg)
{
	uint16_t qmi_ind;
	bool handled = false;
	struct qmi_indication *indication, *tmp;

	if (msg->qmux.service == QMI_SERVICE_CTL)
		qmi_ind = msg->ctl.message;
	else
		qmi_ind = msg->svc.message;


	list_for_each_entry_safe(indication, tmp, &service->indications, list) {
		if (qmi_ind != indication->qmi_ind)
			continue;

		if (indication->cb) {
			indication->cb(service, msg, indication->cb_data);
			handled = true;
		}
	}

	return handled ? 0 : 1;
}
