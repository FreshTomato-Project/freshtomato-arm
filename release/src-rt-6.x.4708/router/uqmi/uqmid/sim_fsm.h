#ifndef __UQMID_SIM_FSM_H
#define __UQMID_SIM_FSM_H

#include <stdbool.h>

enum sim_fsm_state {
	SIM_ST_IDLE,
	SIM_ST_WAIT_UIM_PRESENT,
	SIM_ST_GET_INFO,
	SIM_ST_CHV_PIN,
	SIM_ST_CHV_PUK,
	SIM_ST_READY,
	SIM_ST_FAILED,
	SIM_ST_NO_PIN_TRIES_LEFT,
	SIM_ST_FAIL_PUK_REQUIRED,
	SIM_ST_FAIL_PIN_REQUIRED,
	SIM_ST_FAIL_NO_SIM_PRESENT,
	SIM_ST_DESTROY,
};

// check uim available (or not configured to be used)
// uim get slot status, uim get card status
// uim check if pin is required, check if puk is required
// uim get IMSI, ICCID, MSISDN?
// setup indication to be notified if sim got removed

// sending information towards modem_fsm-> (removed, ready)

enum sim_fsm_event {
	SIM_EV_REQ_START,
	SIM_EV_REQ_CONFIGURED,
	SIM_EV_REQ_DESTROY,

	SIM_EV_RX_UIM_FAILED, /* UIM failed, need to switch to DMS */

	SIM_EV_RX_UIM_GET_SLOT_FAILED, /* some modem doesn't support GET SLOT */
	SIM_EV_RX_UIM_NO_UIM_FOUND, /* we couldn't find a UIM */
	SIM_EV_RX_UIM_VALID_ICCID, /* when Get Slot Status found a valid ICCID */
	SIM_EV_RX_UIM_CARD_STATUS_VALID, /* Got a valid Get Card Status */

	SIM_EV_RX_UIM_GET_IMSI_SUCCESS,
	SIM_EV_RX_UIM_GET_IMSI_FAILED,

	SIM_EV_RX_UIM_PIN_REQUIRED,
	SIM_EV_RX_UIM_PIN_INVALID,
	SIM_EV_RX_UIM_PIN_VERIFIED,

	SIM_EV_RX_UIM_PUK_REQUIRED,
	SIM_EV_RX_UIM_PUK_INVALID,
	SIM_EV_RX_UIM_PUK_VERIFIED,

	SIM_EV_RX_UIM_REFRESH, /* we need to re-check the UIM. E.g. this can happen when pin verified or puk applied, but with a result which we can't parse */
	SIM_EV_RX_UIM_READY,

	SIM_EV_RX_FAILED,
	SIM_EV_RX_SUCCEED, /* a generic callback succeeded */
};

struct modem;
struct osmo_fsm_inst;

void sim_fsm_start(struct modem *modem);
struct osmo_fsm_inst *sim_fsm_alloc(struct modem *modem);

#endif /* __UQMID_SIM_FSM_H */
