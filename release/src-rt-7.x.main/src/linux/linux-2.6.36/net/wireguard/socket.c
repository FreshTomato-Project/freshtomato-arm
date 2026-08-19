// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "device.h"
#include "peer.h"
#include "socket.h"
#include "queueing.h"
#include "messages.h"

#include <linux/ctype.h>
#include <linux/net.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/inetdevice.h>
#include <net/udp_tunnel.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>

static int send4(struct wg_device *wg, struct sk_buff *skb,
		 struct endpoint *endpoint, u8 ds, struct dst_cache *cache)
{
	struct flowi fl = {
		.fl4_src = endpoint->src4.s_addr,
		.fl4_dst = endpoint->addr4.sin_addr.s_addr,
		.fl_ip_dport = endpoint->addr4.sin_port,
		.mark = wg->fwmark,
		.proto = IPPROTO_UDP
	};
	struct rtable *rt = NULL;
	struct sock *sock;
	int ret = 0;

	skb_mark_not_on_list(skb);
	skb->dev = wg->dev;
	skb->mark = wg->fwmark;

	rcu_read_lock_bh();
	sock = rcu_dereference_bh(wg->sock4);

	if (unlikely(!sock)) {
		ret = -ENONET;
		goto err;
	}

	fl.fl_ip_sport = inet_sk(sock)->inet_sport;

	if (cache)
		rt = dst_cache_get_ip4(cache, &fl.fl4_src);

	if (!rt) {
		security_sk_classify_flow(sock, &fl);
		if (unlikely(!inet_confirm_addr(sock_net(sock), NULL, 0,
						fl.fl4_src, RT_SCOPE_HOST))) {
			endpoint->src4.s_addr = 0;
			endpoint->src_if4 = 0;
			fl.fl4_src = 0;
			if (cache)
				dst_cache_reset(cache);
		}
		ip_route_output_flow(sock_net(sock), &rt, &fl, sock, 0);
		if (unlikely(endpoint->src_if4 && ((IS_ERR(rt) &&
			     PTR_ERR(rt) == -EINVAL) || (!IS_ERR(rt) &&
			     rt->dst.dev->ifindex != endpoint->src_if4)))) {
			endpoint->src4.s_addr = 0;
			endpoint->src_if4 = 0;
			fl.fl4_src = 0;
			if (cache)
				dst_cache_reset(cache);
			if (!IS_ERR(rt))
				ip_rt_put(rt);
			ip_route_output_flow(sock_net(sock), &rt, &fl, sock, 0);
		}
		if (IS_ERR(rt)) {
			ret = PTR_ERR(rt);
			net_dbg_ratelimited("%s: No route to %pISpfsc, error %d\n",
					    wg->dev->name, &endpoint->addr, ret);
			goto err;
		}
		if (cache)
			dst_cache_set_ip4(cache, &rt->dst, fl.fl4_src);
	}

	skb->ignore_df = 1;
	udp_tunnel_xmit_skb(rt, sock, skb, fl.fl4_src, fl.fl4_dst, ds,
			    ip4_dst_hoplimit(&rt->dst), 0, fl.fl_ip_sport,
			    fl.fl_ip_dport, false, false);
	goto out;

err:
	kfree_skb(skb);
out:
	rcu_read_unlock_bh();
	return ret;
}

static int send6(struct wg_device *wg, struct sk_buff *skb,
		 struct endpoint *endpoint, u8 ds, struct dst_cache *cache)
{
#if IS_ENABLED(CONFIG_IPV6)
	struct flowi fl = {
		.fl6_src = endpoint->src6,
		.fl6_dst = endpoint->addr6.sin6_addr,
		.fl_ip_dport = endpoint->addr6.sin6_port,
		.mark = wg->fwmark,
		.oif = endpoint->addr6.sin6_scope_id,
		.proto = IPPROTO_UDP
		/* TODO: addr->sin6_flowinfo */
	};
	struct dst_entry *dst = NULL;
	struct sock *sock;
	int ret = 0;

	skb_mark_not_on_list(skb);
	skb->dev = wg->dev;
	skb->mark = wg->fwmark;

	rcu_read_lock_bh();
	sock = rcu_dereference_bh(wg->sock6);

	if (unlikely(!sock)) {
		ret = -ENONET;
		goto err;
	}

	fl.fl_ip_sport = inet_sk(sock)->inet_sport;

	if (cache)
		dst = dst_cache_get_ip6(cache, &fl.fl6_src);

	if (!dst) {
		security_sk_classify_flow(sock, &fl);
		if (unlikely(!ipv6_addr_any(&fl.fl6_src) &&
			     !ipv6_chk_addr(sock_net(sock), &fl.fl6_src, NULL, 0))) {
			endpoint->src6 = fl.fl6_src = in6addr_any;
			if (cache)
				dst_cache_reset(cache);
		}
		dst = ipv6_stub->ipv6_dst_lookup_flow(sock_net(sock), sock, &fl,
						      NULL);
		if (IS_ERR(dst)) {
			ret = PTR_ERR(dst);
			net_dbg_ratelimited("%s: No route to %pISpfsc, error %d\n",
					    wg->dev->name, &endpoint->addr, ret);
			goto err;
		}
		if (cache)
			dst_cache_set_ip6(cache, dst, &fl.fl6_src);
	}

	skb->ignore_df = 1;
	udp_tunnel6_xmit_skb(dst, sock, skb, skb->dev, &fl.fl6_src, &fl.fl6_dst, ds,
			     ip6_dst_hoplimit(dst), 0, fl.fl_ip_sport,
			     fl.fl_ip_dport, false);
	goto out;

err:
	kfree_skb(skb);
out:
	rcu_read_unlock_bh();
	return ret;
#else
	kfree_skb(skb);
	return -EAFNOSUPPORT;
#endif
}

int wg_socket_send_skb_to_peer(struct wg_peer *peer, struct sk_buff *skb, u8 ds)
{
	size_t skb_len = skb->len;
	int ret = -EAFNOSUPPORT;

	read_lock_bh(&peer->endpoint_lock);
	if (peer->endpoint.addr.sa_family == AF_INET)
		ret = send4(peer->device, skb, &peer->endpoint, ds,
			    &peer->endpoint_cache);
	else if (peer->endpoint.addr.sa_family == AF_INET6)
		ret = send6(peer->device, skb, &peer->endpoint, ds,
			    &peer->endpoint_cache);
	else
		dev_kfree_skb(skb);
	if (likely(!ret)) {
		struct net_device *dev = peer->device->dev;

		++dev->stats.tx_packets;
		dev->stats.tx_bytes += skb_len;
		peer->tx_bytes += skb_len;
	}
	read_unlock_bh(&peer->endpoint_lock);

	return ret;
}

int wg_socket_send_buffer_to_peer(struct wg_peer *peer, void *buffer,
				  size_t len, u8 ds)
{
	struct sk_buff *skb = alloc_skb(len + SKB_HEADER_LEN, GFP_ATOMIC);

	if (unlikely(!skb))
		return -ENOMEM;

	skb_reserve(skb, SKB_HEADER_LEN);
	skb_put_data(skb, buffer, len);
	return wg_socket_send_skb_to_peer(peer, skb, ds);
}

int wg_socket_send_buffer_as_reply_to_skb(struct wg_device *wg,
					  struct sk_buff *in_skb, void *buffer,
					  size_t len)
{
	int ret = 0;
	struct sk_buff *skb;
	struct endpoint endpoint;

	if (unlikely(!in_skb))
		return -EINVAL;
	ret = wg_socket_endpoint_from_skb(&endpoint, in_skb);
	if (unlikely(ret < 0))
		return ret;

	skb = alloc_skb(len + SKB_HEADER_LEN, GFP_ATOMIC);
	if (unlikely(!skb))
		return -ENOMEM;
	skb_reserve(skb, SKB_HEADER_LEN);
	skb_put_data(skb, buffer, len);

	if (endpoint.addr.sa_family == AF_INET)
		ret = send4(wg, skb, &endpoint, 0, NULL);
	else if (endpoint.addr.sa_family == AF_INET6)
		ret = send6(wg, skb, &endpoint, 0, NULL);
	/* No other possibilities if the endpoint is valid, which it is,
	 * as we checked above.
	 */

	return ret;
}

int wg_socket_endpoint_from_skb(struct endpoint *endpoint,
				const struct sk_buff *skb)
{
	memset(endpoint, 0, sizeof(*endpoint));
	if (skb->protocol == htons(ETH_P_IP)) {
		endpoint->addr4.sin_family = AF_INET;
		endpoint->addr4.sin_port = udp_hdr(skb)->source;
		endpoint->addr4.sin_addr.s_addr = ip_hdr(skb)->saddr;
		endpoint->src4.s_addr = ip_hdr(skb)->daddr;
		endpoint->src_if4 = skb->skb_iif;
	} else if (IS_ENABLED(CONFIG_IPV6) && skb->protocol == htons(ETH_P_IPV6)) {
		endpoint->addr6.sin6_family = AF_INET6;
		endpoint->addr6.sin6_port = udp_hdr(skb)->source;
		endpoint->addr6.sin6_addr = ipv6_hdr(skb)->saddr;
		endpoint->addr6.sin6_scope_id = ipv6_iface_scope_id(
			&ipv6_hdr(skb)->saddr, skb->skb_iif);
		endpoint->src6 = ipv6_hdr(skb)->daddr;
	} else {
		return -EINVAL;
	}
	return 0;
}

static bool endpoint_eq(const struct endpoint *a, const struct endpoint *b)
{
	return (a->addr.sa_family == AF_INET && b->addr.sa_family == AF_INET &&
		a->addr4.sin_port == b->addr4.sin_port &&
		a->addr4.sin_addr.s_addr == b->addr4.sin_addr.s_addr &&
		a->src4.s_addr == b->src4.s_addr && a->src_if4 == b->src_if4) ||
	       (a->addr.sa_family == AF_INET6 &&
		b->addr.sa_family == AF_INET6 &&
		a->addr6.sin6_port == b->addr6.sin6_port &&
		ipv6_addr_equal(&a->addr6.sin6_addr, &b->addr6.sin6_addr) &&
		a->addr6.sin6_scope_id == b->addr6.sin6_scope_id &&
		ipv6_addr_equal(&a->src6, &b->src6)) ||
	       unlikely(!a->addr.sa_family && !b->addr.sa_family);
}

void wg_socket_set_peer_endpoint(struct wg_peer *peer,
				 const struct endpoint *endpoint)
{
	/* First we check unlocked, in order to optimize, since it's pretty rare
	 * that an endpoint will change. If we happen to be mid-write, and two
	 * CPUs wind up writing the same thing or something slightly different,
	 * it doesn't really matter much either.
	 */
	if (endpoint_eq(endpoint, &peer->endpoint))
		return;
	write_lock_bh(&peer->endpoint_lock);
	if (endpoint->addr.sa_family == AF_INET) {
		peer->endpoint.addr4 = endpoint->addr4;
		peer->endpoint.src4 = endpoint->src4;
		peer->endpoint.src_if4 = endpoint->src_if4;
	} else if (IS_ENABLED(CONFIG_IPV6) && endpoint->addr.sa_family == AF_INET6) {
		peer->endpoint.addr6 = endpoint->addr6;
		peer->endpoint.src6 = endpoint->src6;
	} else {
		goto out;
	}
	dst_cache_reset(&peer->endpoint_cache);
out:
	write_unlock_bh(&peer->endpoint_lock);
}

void wg_socket_set_peer_endpoint_from_skb(struct wg_peer *peer,
					  const struct sk_buff *skb)
{
	struct endpoint endpoint;

	if (!wg_socket_endpoint_from_skb(&endpoint, skb))
		wg_socket_set_peer_endpoint(peer, &endpoint);
}

void wg_socket_clear_peer_endpoint_src(struct wg_peer *peer)
{
	write_lock_bh(&peer->endpoint_lock);
	memset(&peer->endpoint.src6, 0, sizeof(peer->endpoint.src6));
	dst_cache_reset_now(&peer->endpoint_cache);
	write_unlock_bh(&peer->endpoint_lock);
}

static int wg_receive(struct sock *sk, struct sk_buff *skb)
{
	struct wg_device *wg;

	if (unlikely(!sk))
		goto err;
	wg = sk->sk_user_data;
	if (unlikely(!wg))
		goto err;
	skb_mark_not_on_list(skb);
	wg_packet_receive(wg, skb);
	return 0;

err:
	kfree_skb(skb);
	return 0;
}

static void sock_free(struct sock *sock)
{
	if (unlikely(!sock))
		return;
	udp_tunnel_sock_release(sock->sk_socket);
}

static void set_sock_opts(struct socket *sock)
{
	sock->sk->sk_allocation = GFP_ATOMIC;
	sock->sk->sk_sndbuf = INT_MAX;
}

int wg_socket_init(struct wg_device *wg, u16 port)
{
	struct net *net;
	int ret;
	int retries = 0;
#if IS_ENABLED(CONFIG_IPV6)
	bool ipv6_mod, use_ipv6;
	int disable_ipv6_all = -1;
	int disable_ipv6_dflt = -1;
#endif

	struct udp_tunnel_sock_cfg cfg = {
		.sk_user_data = wg,
		.encap_type = 1,
		.encap_rcv = wg_receive
	};
	struct socket *new4 = NULL, *new6 = NULL;
#if IS_ENABLED(CONFIG_IPV6)
	struct udp_port_cfg port6;
#endif
	struct udp_port_cfg port4 = {
		.family = AF_INET,
		.local_udp_port = htons(port),
		.use_udp_checksums = true
	};
	port4.local_ip.s_addr = htonl(INADDR_ANY);
#if IS_ENABLED(CONFIG_IPV6)
	port6.family = AF_INET6;
	port6.use_udp6_tx_checksums = true;
	port6.use_udp6_rx_checksums = true;
	port6.ipv6_v6only = true;
	port6.local_ip6 = in6addr_any;
#endif

	rcu_read_lock();
	net = rcu_dereference(wg->creating_net);
	net = net ? maybe_get_net(net) : NULL;
	rcu_read_unlock();
	if (unlikely(!net))
		return -ENONET;


#if IS_ENABLED(CONFIG_IPV6)
	ipv6_mod = ipv6_mod_enabled();

	if (net->ipv6.devconf_all)
		disable_ipv6_all = net->ipv6.devconf_all->disable_ipv6;
	if (net->ipv6.devconf_dflt)
		disable_ipv6_dflt = net->ipv6.devconf_dflt->disable_ipv6;

	/*
	 * On this legacy kernel ipv6_mod_enabled() is backed by a synthetic
	 * ipv6_stub and does not prove that PF_INET6 is actually registered
	 * or that IPv6 is enabled at runtime.
	 */
	use_ipv6 = ipv6_mod &&
		   net->ipv6.devconf_all &&
		   net->ipv6.devconf_dflt &&
		   !disable_ipv6_all &&
		   !disable_ipv6_dflt;

	printk(KERN_DEBUG "%s: WGDIAG socket init: requested_port=%u "
		"ipv6_mod_enabled=%d disable_ipv6(all/default)=%d/%d "
		"use_ipv6=%d\n",
		wg->dev->name, port, ipv6_mod,
		disable_ipv6_all, disable_ipv6_dflt, use_ipv6);

retry:
#endif

	ret = udp_sock_create(net, &port4, &new4);
	if (ret < 0) {
		pr_err("%s: WGDIAG IPv4 socket create failed: ret=%d\n",
		       wg->dev->name, ret);
		goto out;
	}
	printk(KERN_DEBUG "%s: WGDIAG IPv4 socket ready: port=%u\n",
		wg->dev->name, ntohs(inet_sk(new4->sk)->inet_sport));

	set_sock_opts(new4);
	setup_udp_tunnel_sock(net, new4, &cfg);

#if IS_ENABLED(CONFIG_IPV6)
	if (use_ipv6) {
		port6.local_udp_port = inet_sk(new4->sk)->inet_sport;
		printk(KERN_DEBUG "%s: WGDIAG creating IPv6 socket: port=%u\n",
			wg->dev->name, ntohs(port6.local_udp_port));

		ret = udp_sock_create(net, &port6, &new6);
		if (ret < 0) {
			if (ret == -EADDRINUSE && !port && retries++ < 100) {
				udp_tunnel_sock_release(new4);
				goto retry;
			}

			if (ret == -EAFNOSUPPORT ||
			    ret == -EPROTONOSUPPORT) {
				printk(KERN_DEBUG "%s: WGDIAG IPv6 socket unavailable: "
					"ret=%d; continuing IPv4-only\n",
					wg->dev->name, ret);
				new6 = NULL;
			} else {
				udp_tunnel_sock_release(new4);
				pr_err("%s: WGDIAG IPv6 socket create failed: "
				       "ret=%d port=%u retries=%d\n",
				       wg->dev->name, ret,
				       ntohs(port6.local_udp_port), retries);
				goto out;
			}
		} else {
			printk(KERN_DEBUG "%s: WGDIAG IPv6 socket ready: port=%u\n",
				wg->dev->name,
				ntohs(port6.local_udp_port));
			set_sock_opts(new6);
			setup_udp_tunnel_sock(net, new6, &cfg);
		}
	} else {
		printk(KERN_DEBUG "%s: WGDIAG IPv6 socket skipped; using IPv4-only\n",
			wg->dev->name);
	}
#endif

	wg_socket_reinit(wg, new4->sk, new6 ? new6->sk : NULL);
	ret = 0;
out:
	put_net(net);
	return ret;
}

void wg_socket_reinit(struct wg_device *wg, struct sock *new4,
		      struct sock *new6)
{
	struct sock *old4, *old6;

	mutex_lock(&wg->socket_update_lock);
	old4 = rcu_dereference_protected(wg->sock4,
				lockdep_is_held(&wg->socket_update_lock));
	old6 = rcu_dereference_protected(wg->sock6,
				lockdep_is_held(&wg->socket_update_lock));
	rcu_assign_pointer(wg->sock4, new4);
	rcu_assign_pointer(wg->sock6, new6);
	if (new4)
		wg->incoming_port = ntohs(inet_sk(new4)->inet_sport);
	mutex_unlock(&wg->socket_update_lock);
	synchronize_net();
	sock_free(old4);
	sock_free(old6);
}
