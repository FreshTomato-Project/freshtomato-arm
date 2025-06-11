/*
 * uqmid -- tiny QMI support implementation
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
 */

#include <stdint.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <libqmi-glib.h>
#include "gsmtapv3.h"

/* from osmo-qcdiag under GPLv2 */
/* A small wrapper around libqmi-glib to give us a human-readable string
 * representation of QMI messages that we receive from DIAG */
static int dump_qmi_msg(const uint8_t *data, unsigned int len)
{
	GByteArray *buffer;
	GError *error = NULL;
	QmiMessage *message;
	gchar *printable;

	buffer = g_byte_array_sized_new(len);
	g_byte_array_append(buffer, data, len);

	message = qmi_message_new_from_raw(buffer, &error);
	if (!message) {
		fprintf(stderr, "qmi_message_new_from_raw() returned NULL\n");
		return -1;
	}

	printable = qmi_message_get_printable(message, "QMI ");
	printf("<<<< QMI\n");
	fputs(printable, stdout);
	printf(">>>> QMI\n");
	printf("\n");
	g_free(printable);

	return 0;
}

int main() {
	int sfd, ret;
	struct sockaddr_in in = {};
		sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sfd < 0) {
		fprintf(stderr, "Failed to create socket! %d", ret);
		return -1;
	}

	in.sin_addr.s_addr = 0;
	in.sin_port = htons(GSMTAPV3_UDP_PORT);
	in.sin_family = AF_INET;
	ret = bind(sfd, (struct sockaddr *) &in, sizeof(struct sockaddr_in));
	if (ret < 0) {
		fprintf(stderr, "Failed to bind! %d", ret);
		return -1;
	}

	struct gsmtap_hdr_v3 ghdr;
	uint8_t buffer[4096];
	ssize_t rs;
	void *data;
	while ((rs = recvfrom(sfd, buffer, sizeof(buffer), 0, NULL, 0)) >= 0) {
		uint16_t hdrlen;

		if (rs < sizeof(ghdr))
			continue;

		memcpy(&ghdr, buffer, sizeof(ghdr));
		hdrlen = htons(ghdr.hdr_len) * 4;
		if (hdrlen > rs)
			continue;

		if (ghdr.type != htons(GSMTAPV3_TYPE_BASEBAND_DIAG))
			continue;

		if (ghdr.sub_type != htons(GSMTAPV3_BASEBAND_DIAG_QUALCOMM))
			continue;

		if (hdrlen == rs)
			continue;

		data = buffer + hdrlen;
		dump_qmi_msg(data, rs - hdrlen);
	}
}
