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

#ifndef __UQMID_MODEM_H
#define __UQMID_MODEM_H

#include "sim.h"
#include <libubus.h>
#include <netinet/in.h>

#define PATH_LEN 128

// try to get osmocom fsm into here?
struct modem_config {
	bool configured;
	char *apn;
	char *username;
	char *password;
	char *pin;
	char *puk;
	bool roaming;
	uint8_t pdp_type;
};

struct wwan_conf {
	bool raw_ip;
	bool pass_through;
};


struct modem {
	char *name;
	/*! path to the device. /dev/cdc-wdm */
	char *device;
	/*! IMEI including LUN check digit */
	char *imei;
	/*! 2 digit software version */
	char *imeisv;
	/*! CDMA MEID id - hopefully we never need this */
	char *meid;
	char path[PATH_LEN];

	/* Either usb or usbmisc of cdc-wdm0 */
	char *subsystem_name;
	struct {
		/* network device name. e.g. wwan0 */
		char *dev;
		char *sysfs;
		struct wwan_conf config;

		/* uqmid won't do any sysfs/kernel configuration */
		bool skip_configuration;
	} wwan;

	char *manuf;
	char *model;
	char *rev;

	/* unsure if iccid should be here */
	char *iccid; /* FIXME: iccid length is? */
	char imsi[16];

	struct modem_config config;
	struct {
		/* Radio Access Technology */
		int rat;
		/* Mobile Country Code */
		int mcc;
		/* Mobile Network Code */
		int mnc;
		/* len can be 2 or 3 */
		int mnc_len;
		/* Operator name, can be from Sim or network. */
		char *operator_name;
		/* attached to Circuit Switch/Voice */
		bool cs;
		/* attached to Packet Switch/Data */
		bool ps;
		/* if an error happened and the modem should stay off */
		char *error;
	} state;

	/* TODO: add multiple bearer support later */
	struct {
		/* The QMI internal handle */
		uint32_t packet_data_handle;
		/* either ipv4, ipv6, ipv46 */
		int pdp_type;
		/* valid v4_addr and ipv46 */
		struct sockaddr_in v4_addr;
		struct sockaddr_in v4_netmask;
		struct sockaddr_in v4_gateway;
		/* valid v6 and ipv46 */
		struct sockaddr_in6 v6;
		struct sockaddr_storage dns1;
		struct sockaddr_storage dns2;
	} brearer;

	struct {
		/*! sim_fsm instance */
		struct osmo_fsm_inst *fi;
		/* Do we found a valid simcard */
		bool init;
		/* Certain modems (and maybe simcards) support either unlocking the sim via pin1 or upin. */
		enum uqmi_sim_state state;
		bool use_upin;
		bool use_uim;
		bool requires_unlock;
		/* when we tried the configured pin once */
		bool pin_tried;
		bool puk_tried;
		int pin_retries;
		int puk_retries;
	} sim;

	struct qmi_dev *qmi;

	struct list_head list;
	struct ubus_object ubus;

	/*! modem_fsm instance */
	struct osmo_fsm_inst *fi;
};

int uqmid_modem_add(const char *name, const char *device, bool qmi_over_mbim);
int uqmid_modem_remove(struct modem *modem);
int uqmid_modem_start(struct modem *modem);
int uqmid_modem_configured(struct modem *modem);
void uqmid_modem_set_error(struct modem *modem, const char *error);

typedef void (*uqmid_modem_get_opmode_cb)(void *data, int opmode_err, int opmode);
int uqmid_modem_get_opmode(struct modem *modem, uqmid_modem_get_opmode_cb cb, void *cb_data);
int uqmid_modem_networkstatus(struct modem *modem);
struct modem *uqmid_modem_find_by_device(const char *device);
struct modem *uqmid_modem_find_by_name(const char *name);

#endif /* __UQMID_MODEM_H */
