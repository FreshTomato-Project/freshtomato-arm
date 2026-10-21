/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2016 Felix Fietkau <nbd@openwrt.org>
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
static int uloop_init_pollfd(void)
{
	struct timespec timeout = { 0, 0 };
	struct kevent ev = {};

	if (poll_fd >= 0)
		return 0;

	poll_fd = kqueue();
	if (poll_fd < 0)
		return -1;

	EV_SET(&ev, SIGCHLD, EVFILT_SIGNAL, EV_ADD, 0, 0, 0);
	kevent(poll_fd, &ev, 1, NULL, 0, &timeout);

	return 0;
}


static uint16_t get_flags(unsigned int flags, unsigned int mask)
{
	uint16_t kflags = 0;

	if (!(flags & mask))
		return EV_DELETE;

	kflags = EV_ADD;
	if (flags & ULOOP_EDGE_TRIGGER)
		kflags |= EV_CLEAR;

	return kflags;
}

static struct kevent events[ULOOP_MAX_EVENTS];

static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct timespec timeout = { 0, 0 };
	unsigned int registered = fd->flags;
	struct kevent ev[4];
	int nev = 0;
	unsigned int changed;
	uint16_t kflags;

	/* EV_ADD does not change EV_CLEAR on a filter that already exists */
	if ((flags ^ registered) & ULOOP_EDGE_TRIGGER) {
		if (registered & ULOOP_READ)
			EV_SET(&ev[nev++], fd->fd, EVFILT_READ, EV_DELETE, 0, 0, fd);
		if (registered & ULOOP_WRITE)
			EV_SET(&ev[nev++], fd->fd, EVFILT_WRITE, EV_DELETE, 0, 0, fd);
		registered = 0;
	}

	changed = flags ^ registered;

	/* epoll re-arms an edge triggered descriptor on every EPOLL_CTL_MOD */
	if (flags & ULOOP_EDGE_TRIGGER)
		changed |= flags;

	if (changed & ULOOP_READ) {
		kflags = get_flags(flags, ULOOP_READ);
		EV_SET(&ev[nev++], fd->fd, EVFILT_READ, kflags, 0, 0, fd);
	}

	if (changed & ULOOP_WRITE) {
		kflags = get_flags(flags, ULOOP_WRITE);
		EV_SET(&ev[nev++], fd->fd, EVFILT_WRITE, kflags, 0, 0, fd);
	}

	if (!nev)
		return 0;

	if (kevent(poll_fd, ev, nev, NULL, 0, &timeout) == -1)
		return -1;

	return 0;
}

static int __uloop_fd_delete(struct uloop_fd *fd)
{
	return register_poll(fd, 0);
}

static int64_t get_timestamp_us(void)
{
#ifdef CLOCK_MONOTONIC
	struct timespec ts = { 0, 0 };

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#else
	struct timeval tv = { 0, 0 };

	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

static bool fd_hangup(int fd)
{
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	struct stat st;

	if (fstat(fd, &st) || !S_ISSOCK(st.st_mode))
		return true;

	/* fails once both directions of the connection are shut down */
	return getpeername(fd, (struct sockaddr *)&ss, &len) != 0;
}

static bool kevent_is_error(struct kevent *kev, struct uloop_fd *u)
{
	if (kev->flags & EV_ERROR)
		return true;

	if (!(kev->flags & EV_EOF))
		return false;

	/*
	 * fflags holds a pending socket error. Otherwise, EOF is a hangup
	 * (EPOLLHUP) unless only the peer of a socket stopped sending
	 * (EPOLLRDHUP).
	 */
	return kev->fflags || fd_hangup(u->fd);
}

/*
 * kqueue reports read and write readiness as separate events, epoll as one.
 * Merge them, so that a callback runs once per descriptor and batch.
 */
static struct uloop_fd_event *uloop_fd_event_get(struct uloop_fd *u, int *nev)
{
	struct uloop_fd_event *cur;
	int i;

	for (i = 0; i < *nev; i++)
		if (cur_fds[i].fd == u)
			return &cur_fds[i];

	cur = &cur_fds[(*nev)++];
	cur->fd = u;
	cur->events = 0;

	return cur;
}

struct uloop_timer_batch {
	struct uloop_timer_batch *next;
	struct uloop_interval *tm[ULOOP_MAX_EVENTS];
	int64_t expirations[ULOOP_MAX_EVENTS];
	int n;
};

/* batches whose callbacks run, innermost first; a callback may nest uloop_run() */
static struct uloop_timer_batch *timer_batches;

static void uloop_timer_batch_run(struct uloop_timer_batch *batch)
{
	struct uloop_interval *tm;
	int i;

	batch->next = timer_batches;
	timer_batches = batch;

	for (i = 0; i < batch->n; i++) {
		tm = batch->tm[i];
		if (!tm)
			continue;

		tm->priv.time.fired = get_timestamp_us();
		tm->expirations += batch->expirations[i];
		tm->cb(tm);
	}

	timer_batches = batch->next;
}

static int uloop_fetch_events(int timeout)
{
	struct uloop_timer_batch batch = {};
	struct timespec ts;
	int nfds, nev = 0, n;

	if (timeout >= 0) {
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;
	}

	nfds = kevent(poll_fd, NULL, 0, events, ARRAY_SIZE(events), timeout >= 0 ? &ts : NULL);
	for (n = 0; n < nfds; n++) {
		struct kevent *kev = &events[n];
		struct uloop_fd_event *cur;
		struct uloop_fd *u;

		if (kev->filter == EVFILT_TIMER) {
			batch.tm[batch.n] = kev->udata;
			batch.expirations[batch.n++] = kev->data;
			continue;
		}

		u = kev->udata;
		if (!u || (kev->filter != EVFILT_READ && kev->filter != EVFILT_WRITE))
			continue;

		cur = uloop_fd_event_get(u, &nev);
		cur->events |= kev->filter == EVFILT_READ ? ULOOP_READ : ULOOP_WRITE;

		if (kev->flags & EV_EOF)
			u->eof = true;

		if (kevent_is_error(kev, u)) {
			u->error = true;
			if (!(u->flags & ULOOP_ERROR_CB))
				uloop_fd_delete(u);
		}
	}

	/*
	 * Publish the descriptor events before the interval callbacks run, so
	 * that uloop_fd_delete() from a callback drops the pending event. A
	 * nested uloop_run() may consume some of them, so return what is left.
	 */
	cur_nfds = nev;
	uloop_timer_batch_run(&batch);

	return cur_nfds;
}

static int timer_register(struct uloop_interval *tm, unsigned int msecs)
{
	struct kevent ev;

	tm->priv.time.msecs = msecs;
	tm->priv.time.fired = get_timestamp_us();

	EV_SET(&ev, (uintptr_t)tm, EVFILT_TIMER, EV_ADD, NOTE_USECONDS,
	       (int64_t)msecs * 1000, tm);

	return kevent(poll_fd, &ev, 1, NULL, 0, NULL);
}

static int timer_remove(struct uloop_interval *tm)
{
	struct uloop_timer_batch *batch;
	struct kevent ev;
	int i;

	for (batch = timer_batches; batch; batch = batch->next)
		for (i = 0; i < batch->n; i++)
			if (batch->tm[i] == tm)
				batch->tm[i] = NULL;

	EV_SET(&ev, (uintptr_t)tm, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);

	return kevent(poll_fd, &ev, 1, NULL, 0, NULL);
}

static int64_t timer_next(struct uloop_interval *tm)
{
	int64_t t1 = tm->priv.time.fired;
	int64_t t2 = get_timestamp_us();
	int64_t step = (int64_t)tm->priv.time.msecs * 1000;

	if (step <= 0)
		return 0;

	while (t1 < t2)
		t1 += step;

	return (t1 - t2) / 1000;
}
