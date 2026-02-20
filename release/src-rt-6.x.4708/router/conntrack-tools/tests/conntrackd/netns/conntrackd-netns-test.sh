#!/bin/bash

if [ $UID -ne 0 ]
then
	echo "You must be root to run this test script"
	exit 0
fi

start () {
	ip netns add ns1
	ip netns add ns2
	ip netns add nsr1
	ip netns add nsr2

	ip link add veth0 netns ns1 type veth peer name veth1 netns nsr1
	ip link add veth0 netns nsr1 type veth peer name veth0 netns ns2
	ip link add veth2 netns nsr1 type veth peer name veth0 netns nsr2

	ip -net ns1 addr add 192.168.10.2/24 dev veth0
	ip -6 -net ns1 addr add bbbb::2/64 dev veth0
	ip -net ns1 link set up dev veth0
	ip -net ns1 ro add 10.0.1.0/24 via 192.168.10.1 dev veth0
	ip -6 -net ns1 ro add aaaa::/64 via bbbb::1 dev veth0

	ip -net nsr1 addr add 10.0.1.1/24 dev veth0
	ip -net nsr1 addr add 192.168.10.1/24 dev veth1
	ip -6 -net nsr1 addr add aaaa::1/64 dev veth0
	ip -6 -net nsr1 addr add bbbb::1/64 dev veth1
	ip -net nsr1 link set up dev veth0
	ip -net nsr1 link set up dev veth1
	ip -net nsr1 route add default via 192.168.10.2
	ip -6 -net nsr1 route add default via bbbb::2
	ip netns exec nsr1 sysctl net.ipv4.ip_forward=1
	ip netns exec nsr1 sysctl net.ipv6.conf.all.forwarding=1

	ip -net nsr1 addr add 192.168.100.2/24 dev veth2
	ip -6 -net nsr1 addr add cccc::2/96 dev veth2
	ip -net nsr1 link set up dev veth2
	ip -net nsr2 addr add 192.168.100.3/24 dev veth0
	ip -6 -net nsr2 addr add cccc::3/96 dev veth0
	ip -net nsr2 link set up dev veth0

	ip -net ns2 addr add 10.0.1.2/24 dev veth0
	ip -6 -net ns2 addr add aaaa::2/64 dev veth0
	ip -net ns2 link set up dev veth0
	ip -net ns2 route add default via 10.0.1.1
	ip -6 -net ns2 route add default via aaaa::1

	echo 1 > /proc/sys/net/netfilter/nf_log_all_netns

	ip netns exec nsr1 nft -f ruleset-nsr1.nft
	ip netns exec nsr1 conntrackd -C conntrackd-nsr1.conf -d
	ip netns exec nsr2 conntrackd -C conntrackd-nsr2.conf -d
}

stop () {
	ip netns del ns1
	ip netns del ns2
	ip netns del nsr1
	ip netns del nsr2
	killall -15 conntrackd
}

case $1 in
start)
	start
	;;
stop)
	stop
	;;
*)
	echo "$0 [start|stop]"
	;;
esac

exit 0
