#ifndef __UQMID_MODEM_TX_H
#define __UQMID_MODEM_TX_H

#include "qmi-enums-uim.h"
#include "uqmid.h"

#include <stdint.h>

struct modem;
struct qmi_service;

int tx_dms_set_operating_mode(struct modem *modem, struct qmi_service *dms, uint8_t operating_mode, request_cb cb);
int tx_nas_subscribe_nas_events(struct modem *modem, struct qmi_service *nas, bool action, request_cb cb);

int tx_uim_read_transparent_file(struct modem *modem, struct qmi_service *wds, request_cb cb,
                                 uint16_t file_id, uint8_t *filepath, unsigned int filepath_n);
int tx_uim_verify_pin(struct modem *modem, struct qmi_service *uim, request_cb cb,
					  QmiUimPinId pin_id, char *pin_value);

int tx_uim_unblock_pin(struct modem *modem, struct qmi_service *uim, request_cb cb,
					   QmiUimPinId pin_id, char *new_pin_value, char *puk_value);

int tx_wda_set_data_format(struct modem *modem, struct qmi_service *wda, request_cb cb);
int tx_wds_get_profile_list(struct modem *modem, struct qmi_service *wds, request_cb cb);
int tx_wds_modify_profile(struct modem *modem, struct qmi_service *wds, request_cb cb, uint8_t profile, const char *apn,
			  uint8_t pdp_type, const char *username, const char *password);
int tx_wds_start_network(struct modem *modem, struct qmi_service *wds, request_cb cb, uint8_t profile_idx,
			 uint8_t ip_family);
int tx_wds_stop_network(struct modem *modem, struct qmi_service *wds, request_cb cb, uint32_t packet_data_handle,
			bool *disable_autoconnect);
int tx_wds_get_current_settings(struct modem *modem, struct qmi_service *wds, request_cb cb);

#endif /* __UQMID_MODEM_TX_H */
