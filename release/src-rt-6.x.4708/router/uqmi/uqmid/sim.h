/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2024 Alexander Couzens <lynxis@fe80.eu>
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

#ifndef __QMID_SIM_H
#define __QMID_SIM_H

#include <stdint.h>

enum uqmi_sim_state {
	UQMI_SIM_UNKNOWN = 0,
	UQMI_SIM_PIN_REQUIRED, /* Pin1 required */
	UQMI_SIM_PUK_REQUIRED, /* Puk1 required */
	UQMI_SIM_READY, /* Ready to operate, either no pin required or already enterd */
	UQMI_SIM_BLOCKED, /* Blocked without knowing how to unlock. */
};

enum uqmi_sim_state uim_card_application_state_to_uqmi_state(int app_state);
enum uqmi_sim_state uim_pin_to_uqmi_state(int upin_state);

int uqmi_sim_decode_imsi(uint8_t *imsi_ef, uint8_t imsi_ef_size, char *imsi_str, uint8_t imsi_str_len);

#endif /* __UTILS_H */
