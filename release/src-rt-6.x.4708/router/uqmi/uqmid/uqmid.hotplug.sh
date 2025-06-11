#!/bin/sh
# based on wwan usbmisc
# install it to /etc/hotplug.d/usbmisc/01_uqmid.sh

[ "$ACTION" = add ] || [ "$ACTION" = remove ] || exit 0
[ "${DEVNAME/[0-9]/}" = cdc-wdm ] || exit 0

. /lib/functions.sh
. /lib/netifd/netifd-proto.sh

find_uqmid_iface() {
	local cfg="$1"

	local proto device
	config_get proto "$cfg" proto
	config_get device "$cfg" device

	[ "$proto" = uqmid ] || return 0
	[ "$device" = "/dev/$DEVNAME" ] || return 0
	if [ "$ACTION" = add ]; then
		proto_set_available "$cfg" 1
	fi
	if [ "$ACTION" = remove ]; then
		proto_set_available "$cfg" 0
	fi
	exit 0
}

config_load network
config_foreach find_uqmid_iface interface
