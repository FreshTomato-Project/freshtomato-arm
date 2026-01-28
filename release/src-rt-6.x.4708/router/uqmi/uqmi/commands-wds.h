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

#define __uqmi_wds_commands \
	__uqmi_command(wds_start_network, start-network, no, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_apn, apn, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_auth, auth-type, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_username, username, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_password, password, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_ip_family_pref, ip-family, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_autoconnect, autoconnect, no, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_profile, profile, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_stop_network, stop-network, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_get_packet_service_status, get-data-status, no, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_ip_family, set-ip-family, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_autoconnect_settings, set-autoconnect, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_reset, reset-wds, no, QMI_SERVICE_WDS), \
	__uqmi_command(wds_get_profile_settings, get-profile-settings, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_default_profile, set-default-profile, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_get_default_profile, get-default-profile, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_get_profile_list, get-profile-list, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_create_profile, create-profile, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_modify_profile, modify-profile, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_delete_profile, delete-profile, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_pdp_type, pdp-type, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_no_roaming, no-roaming, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_get_current_settings, get-current-settings, no, QMI_SERVICE_WDS), \
	__uqmi_command(wds_bind_mux, bind-mux, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_ep_type, endpoint-type, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_ep_iface, endpoint-iface, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_lte_attach_pdn, lte-attach-pdn, required, QMI_SERVICE_WDS)


#define wds_helptext \
		"  --start-network:                  Start network connection (use with options below)\n" \
		"    --apn <apn>:                    Use APN\n" \
		"    --auth-type pap|chap|both|none: Use network authentication type\n" \
		"    --username <name>:              Use network username\n" \
		"    --password <password>:          Use network password\n" \
		"    --ip-family <family>:           Use ip-family for the connection (ipv4, ipv6, unspecified)\n" \
		"    --autoconnect:                  Enable automatic connect/reconnect\n" \
		"    --profile <index>:              Use connection profile\n" \
		"  --stop-network <pdh>:             Stop network connection (use with option below)\n" \
		"    --autoconnect:                  Disable automatic connect/reconnect\n" \
		"  --get-data-status:                Get current data access status\n" \
		"  --set-ip-family <val>:            Set ip-family (ipv4, ipv6, unspecified)\n" \
		"  --set-autoconnect <val>:          Set automatic connect/reconnect (disabled, enabled, paused)\n" \
		"  --get-profile-settings <val,#>:   Get APN profile settings (3gpp, 3gpp2),#\n" \
		"  --set-default-profile <val,#>:    Set default profile number (3gpp, 3gpp2)\n" \
		"  --get-default-profile <val>:      Get default profile number (3gpp, 3gpp2)\n" \
		"  --get-profile-list <val>:         Get List of profiles (3gpp, 3gpp2)\n" \
		"  --create-profile <val>            Create profile (3gpp, 3gpp2)\n" \
		"    --apn <apn>:                    Use APN\n" \
		"    --pdp-type ipv4|ipv6|ipv4v6>:   Use pdp-type for the connection\n" \
		"    --username <name>:              Use network username\n" \
		"    --password <password>:          Use network password\n" \
		"    --auth-type pap|chap|both|none: Use network authentication type\n" \
		"    --no-roaming false|true         To allow roaming, set to false\n" \
		"  --modify-profile <val>,#          Modify profile number (3gpp, 3gpp2)\n" \
		"    --apn <apn>:                    Use APN\n" \
		"    --pdp-type ipv4|ipv6|ipv4v6>:   Use pdp-type for the connection\n" \
		"    --username <name>:              Use network username\n" \
		"    --password <password>:          Use network password\n" \
		"    --auth-type pap|chap|both|none: Use network authentication type\n" \
		"    --no-roaming false|true         To allow roaming, set to false\n" \
		"  --delete-profile <val>,#:         Delete profile number (3gpp, 3gpp2)\n" \
		"  --get-current-settings:           Get current connection settings\n" \
		"  --bind-mux <id>:                  Bind data session to QMAP multiplex (use with options below)\n" \
		"    --endpoint-type <type>:         Set endpoint interface type (hsusb, pcie)\n" \
		"    --endpoint-iface <number>:      Set endpoint interface number\n" \
		"  --set-lte-attach-pdn #,#,...:     Set list of PDN connections used when attaching to LTE/5G\n"
