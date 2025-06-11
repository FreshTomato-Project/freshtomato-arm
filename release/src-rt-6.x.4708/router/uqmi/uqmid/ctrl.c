
#include <stdint.h>
#include <talloc.h>

#include "services.h"
#include "uqmid.h"

#include "ctrl.h"
#include "qmi-message.h"

static void uqmi_ctrl_request_clientid_cb(struct qmi_service *ctrl, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_ctl_allocate_cid_response res;
	struct qmi_service *service;
	if (!msg)
		return;

	qmi_parse_ctl_allocate_cid_response(msg, &res);
	service = uqmi_service_find(ctrl->qmi, res.data.allocation_info.service);
	if (!service) {
		/* FIXME: error log("Can't find the service for the allocated CID") */
		return;
	}

	uqmi_service_get_client_id_cb(service, res.data.allocation_info.cid);
}

int uqmi_ctrl_request_clientid(struct qmi_service *service)
{
	struct qmi_service *ctrl = service->qmi->ctrl;
	struct qmi_request *req = talloc_zero(ctrl, struct qmi_request);
	struct qmi_msg *msg = talloc_zero_size(req, 128);


	struct qmi_ctl_allocate_cid_request creq = {
		QMI_INIT(service, service->service)
	};
	qmi_set_ctl_allocate_cid_request(msg, &creq);
	req->cb = uqmi_ctrl_request_clientid_cb;
	req->msg = msg;

	return uqmi_service_send_msg(ctrl, req);
}

static void uqmi_ctrl_release_clientid_cb(struct qmi_service *ctrl, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_ctl_release_cid_response res;
	struct qmi_service *service;

	if (!msg)
		return;

	if (qmi_parse_ctl_release_cid_response(msg, &res)) {
		/* error_log("Couldn't parse release cid response") */
		return;
	}

	if (!res.set.release_info)
		return;

	service = uqmi_service_find(ctrl->qmi, res.data.release_info.service);
	if (service && service->service)
		uqmi_service_close_cb(service);
}

int uqmi_ctrl_release_clientid(struct qmi_service *service)
{
	struct qmi_service *ctrl = service->qmi->ctrl;
	struct qmi_request *req = talloc_zero(ctrl, struct qmi_request);
	struct qmi_msg *msg = talloc_zero_size(req, 128);

	struct qmi_ctl_release_cid_request creq = {
		QMI_INIT_SEQUENCE(release_info,
			.service = service->service,
			.cid = service->client_id,
		)
	};
	qmi_set_ctl_release_cid_request(msg, &creq);
	req->msg = msg;
	req->cb = uqmi_ctrl_release_clientid_cb;

	return uqmi_service_send_msg(ctrl, req);
}

struct qmi_service *uqmi_ctrl_generate(struct qmi_dev *qmi)
{
	qmi->ctrl = uqmi_service_create(qmi, 0);
	qmi->ctrl->client_id = 0;
	qmi->ctrl->state = SERVICE_READY;

	return qmi->ctrl;
}

