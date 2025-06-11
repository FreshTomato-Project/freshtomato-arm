/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>

#include <talloc.h>
#include <unistd.h>
#include "gsmtapv3.h"

#include "gsmtap_util.h"

struct t16l16v {
	uint16_t type;
	uint16_t length;
	uint8_t value[0];
} __attribute__((packed));

#define GSMTAPV3_BB_DIAG_QC_QMI 1

struct gsmtap_instance {
	int fd;
	bool valid;
};

static struct gsmtap_instance gsmtap_inst;

void gsmtap_disable(void)
{
	if (!gsmtap_inst.valid)
		return;

	if (gsmtap_inst.fd >= 0) {
		close(gsmtap_inst.fd);
		gsmtap_inst.fd = -1;
	}

	gsmtap_inst.valid = false;
}

/* add support for IPv6 */
int gsmtap_enable(const char *gsmtap_addr)
{
	int ret, sfd;
	struct sockaddr_in in = {};

	gsmtap_disable();
	sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sfd < 0)
		return -1;

	in.sin_addr.s_addr = inet_addr(gsmtap_addr);
	in.sin_port = htons(GSMTAPV3_UDP_PORT);
	in.sin_family = AF_INET;
	ret = connect(sfd, (struct sockaddr *) &in, sizeof(struct sockaddr_in));
	if (ret < 0)
		return -1;

	gsmtap_inst.fd = sfd;
	gsmtap_inst.valid = true;

	return 0;
}

static void tx_gsmtap(void *msg, size_t length)
{
	if (!gsmtap_inst.valid)
		return ;

	write(gsmtap_inst.fd, msg, length);
}

void gsmtap_send(struct modem *modem, void *data, size_t length)
{
	if (!gsmtap_inst.valid)
		return;

	/* WARNING. GSMTAPv3 is still under development and defines will change! */
	void *user_data;
	void *msg = talloc_size(modem, length + sizeof(struct gsmtap_hdr_v3) + 32);
	struct gsmtap_hdr_v3 *gsmtap = msg;
	struct t16l16v *metadata = msg + sizeof(struct gsmtap_hdr_v3);
	int meta_len = 0, rest;
	size_t packet_len = 0;

	gsmtap->version = GSMTAPV3_VERSION;
	gsmtap->res = 0;
	gsmtap->hdr_len = sizeof(struct gsmtap_hdr_v3) >> 2;
	gsmtap->type = htons(GSMTAPV3_TYPE_BASEBAND_DIAG);
	gsmtap->sub_type = htons(GSMTAPV3_BASEBAND_DIAG_QUALCOMM);

	metadata->type = 0x1;
	metadata->length = 0x1;
	metadata->value[0] = 0x1;

	meta_len = sizeof(*metadata) + metadata->length;
	rest = meta_len % 4;
	if (rest)
		meta_len += 4 - rest;

	gsmtap->hdr_len += (meta_len >> 2);
	gsmtap->hdr_len = htons(gsmtap->hdr_len);

	user_data = msg + sizeof(struct gsmtap_hdr_v3) + meta_len;
	memcpy(user_data, data, length);

	packet_len = length + sizeof(struct gsmtap_hdr_v3) + meta_len;
	tx_gsmtap(msg, packet_len);
}
