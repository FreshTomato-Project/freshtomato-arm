/*
 * usock - socket helper functions
 *
 * Copyright (C) 2010 Steven Barth <steven@midlink.org>
 * Copyright (C) 2011-2012 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <poll.h>

#include "usock.h"
#include "utils.h"

static void usock_set_flags(int sock, unsigned int type)
{
	if (!(type & USOCK_NOCLOEXEC))
		fcntl(sock, F_SETFD, fcntl(sock, F_GETFD) | FD_CLOEXEC);

	if (type & USOCK_NONBLOCK)
		fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
}

static int usock_connect(int type, struct sockaddr *sa, int sa_len, int family, int socktype, bool server)
{
	int sock;

	sock = socket(family, socktype, 0);
	if (sock < 0)
		return -1;

	usock_set_flags(sock, type);

	if (server) {
		const int one = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (!bind(sock, sa, sa_len) &&
		    (socktype != SOCK_STREAM || !listen(sock, SOMAXCONN)))
			return sock;
	} else {
		if (!connect(sock, sa, sa_len) || errno == EINPROGRESS)
			return sock;
	}

	close(sock);
	return -1;
}

static int usock_unix(int type, const char *host)
{
	struct sockaddr_un sun = {.sun_family = AF_UNIX};
	bool server = !!(type & USOCK_SERVER);
	int socktype = ((type & 0xff) == USOCK_TCP) ? SOCK_STREAM : SOCK_DGRAM;

	if (strlen(host) >= sizeof(sun.sun_path)) {
		errno = EINVAL;
		return -1;
	}
	strcpy(sun.sun_path, host);

	return usock_connect(type, (struct sockaddr*)&sun, sizeof(sun), AF_UNIX, socktype, server);
}

static int
usock_inet_notimeout(int type, struct addrinfo *result, void *addr)
{
	struct addrinfo *rp;
	int socktype = ((type & 0xff) == USOCK_TCP) ? SOCK_STREAM : SOCK_DGRAM;
	bool server = !!(type & USOCK_SERVER);
	int sock;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = usock_connect(type, rp->ai_addr, rp->ai_addrlen, rp->ai_family, socktype, server);
		if (sock >= 0) {
			if (addr)
				memcpy(addr, rp->ai_addr, rp->ai_addrlen);
			return sock;
		}
	}

	return -1;
}

static int poll_restart(struct pollfd *fds, int nfds, int timeout)
{
	struct timespec ts, cur;
	int msec = timeout % 1000;
	int ret;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	ts.tv_nsec += msec * 1000000;
	if (ts.tv_nsec > 1000000000) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}
	ts.tv_sec += timeout / 1000;

	while (1) {
		ret = poll(fds, nfds, timeout);
		if (ret >= 0 || (errno != EINTR && errno != EAGAIN))
			return ret;

		clock_gettime(CLOCK_MONOTONIC, &cur);
		timeout = (ts.tv_sec - cur.tv_sec) * 1000;
		timeout += (ts.tv_nsec - cur.tv_nsec) / 1000000;
		if (timeout <= 0)
			return 0;
	}
}

static int usock_check_connect(int fd)
{
	int err = 0;
	socklen_t len = sizeof(err);

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len))
		return -1;

	return err ? -1 : 0;
}

static int usock_timeout_remaining(struct timespec *deadline)
{
	struct timespec cur;
	int msec;

	clock_gettime(CLOCK_MONOTONIC, &cur);
	msec = (deadline->tv_sec - cur.tv_sec) * 1000;
	msec += (deadline->tv_nsec - cur.tv_nsec) / 1000000;

	return msec > 0 ? msec : 0;
}

#define USOCK_MAX_CANDIDATES 8
#define USOCK_CONNECT_DELAY_MS 250

static int usock_addr_interleave(struct addrinfo *result,
				 struct addrinfo **candidates, int max)
{
	struct addrinfo *v6[USOCK_MAX_CANDIDATES], *v4[USOCK_MAX_CANDIDATES];
	struct addrinfo *rp;
	int n_v6 = 0, n_v4 = 0, n = 0, i;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_family == AF_INET6 && n_v6 < USOCK_MAX_CANDIDATES)
			v6[n_v6++] = rp;
		else if (rp->ai_family == AF_INET && n_v4 < USOCK_MAX_CANDIDATES)
			v4[n_v4++] = rp;
	}

	for (i = 0; n < max && (i < n_v6 || i < n_v4); i++) {
		if (i < n_v6 && n < max)
			candidates[n++] = v6[i];
		if (i < n_v4 && n < max)
			candidates[n++] = v4[i];
	}

	return n;
}

int usock_inet_timeout(int type, const char *host, const char *service,
		       void *addr, int timeout)
{
	int socktype = ((type & 0xff) == USOCK_TCP) ? SOCK_STREAM : SOCK_DGRAM;
	bool server = !!(type & USOCK_SERVER);
	struct addrinfo *result, *rp;
	struct addrinfo hints = {
		.ai_family = (type & USOCK_IPV6ONLY) ? AF_INET6 :
			(type & USOCK_IPV4ONLY) ? AF_INET : AF_UNSPEC,
		.ai_socktype = socktype,
		.ai_flags = AI_ADDRCONFIG
			| ((type & USOCK_SERVER) ? AI_PASSIVE : 0)
			| ((type & USOCK_NUMERIC) ? AI_NUMERICHOST : 0),
	};
	struct addrinfo *candidates[USOCK_MAX_CANDIDATES];
	struct addrinfo *pfd_ai[USOCK_MAX_CANDIDATES];
	struct pollfd pfds[USOCK_MAX_CANDIDATES];
	struct timespec deadline;
	int n_candidates, n_active = 0;
	int sock = -1;
	int fd, delay, i, j;

	if (getaddrinfo(host, service, &hints, &result))
		return -1;

	if (timeout <= 0 || server) {
		sock = usock_inet_notimeout(type, result, addr);
		goto free_addrinfo;
	}

	clock_gettime(CLOCK_MONOTONIC, &deadline);
	deadline.tv_nsec += (timeout % 1000) * 1000000;
	if (deadline.tv_nsec >= 1000000000) {
		deadline.tv_sec++;
		deadline.tv_nsec -= 1000000000;
	}
	deadline.tv_sec += timeout / 1000;

	n_candidates = usock_addr_interleave(result, candidates,
					     USOCK_MAX_CANDIDATES);
	if (!n_candidates)
		goto out;

	for (i = 0; i < n_candidates; i++) {
		rp = candidates[i];
		fd = usock_connect(type | USOCK_NONBLOCK, rp->ai_addr,
				   rp->ai_addrlen, rp->ai_family,
				   socktype, server);
		if (fd < 0)
			continue;

		pfds[n_active] = (struct pollfd){ .fd = fd, .events = POLLOUT };
		pfd_ai[n_active] = rp;
		n_active++;

		delay = usock_timeout_remaining(&deadline);
		if (delay <= 0)
			break;
		if (i < n_candidates - 1 && delay > USOCK_CONNECT_DELAY_MS)
			delay = USOCK_CONNECT_DELAY_MS;

		poll_restart(pfds, n_active, delay);

		for (j = n_active - 1; j >= 0; j--) {
			if (!(pfds[j].revents & POLLOUT))
				continue;

			if (usock_check_connect(pfds[j].fd) == 0) {
				sock = pfds[j].fd;
				rp = pfd_ai[j];
				goto out;
			}

			close(pfds[j].fd);
			n_active--;
			pfds[j] = pfds[n_active];
			pfd_ai[j] = pfd_ai[n_active];
		}
	}

	while (n_active > 0) {
		delay = usock_timeout_remaining(&deadline);
		if (delay <= 0)
			break;

		poll_restart(pfds, n_active, delay);

		for (j = n_active - 1; j >= 0; j--) {
			if (!(pfds[j].revents & POLLOUT))
				continue;

			if (usock_check_connect(pfds[j].fd) == 0) {
				sock = pfds[j].fd;
				rp = pfd_ai[j];
				goto out;
			}

			close(pfds[j].fd);
			n_active--;
			pfds[j] = pfds[n_active];
			pfd_ai[j] = pfd_ai[n_active];
		}
	}

out:
	for (j = 0; j < n_active; j++) {
		if (pfds[j].fd != sock)
			close(pfds[j].fd);
	}

	if (sock >= 0) {
		if (!(type & USOCK_NONBLOCK))
			fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) & ~O_NONBLOCK);
		if (addr)
			memcpy(addr, rp->ai_addr, rp->ai_addrlen);
	}
free_addrinfo:
	freeaddrinfo(result);
	return sock;
}

const char *usock_port(int port)
{
	static char buffer[sizeof("65535\0")];

	if (port < 0 || port > 65535)
		return NULL;

	snprintf(buffer, sizeof(buffer), "%u", port);

	return buffer;
}

int usock(int type, const char *host, const char *service) {
	int sock;

	if (type & USOCK_UNIX)
		sock = usock_unix(type, host);
	else
		sock = usock_inet(type, host, service, NULL);

	if (sock < 0)
		return -1;

	return sock;
}

int usock_wait_ready(int fd, int msecs) {
	struct pollfd fds[1];
	int res;

	fds[0].fd = fd;
	fds[0].events = POLLOUT;

	res = poll(fds, 1, msecs);
	if (res < 0) {
		return errno;
	} else if (res == 0) {
		return -ETIMEDOUT;
	} else {
		int err = 0;
		socklen_t optlen = sizeof(err);

		res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &optlen);
		if (res)
			return errno;
		if (err)
			return err;
	}

	return 0;
}
