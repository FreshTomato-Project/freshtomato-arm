/*
 * uqmid -- implement a daemon based on the uqmi idea
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2023 Alexander Couzens <lynxis@fe80.eu>
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

#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>

#include "uqmid.h"
#include "ubus.h"

static const struct option uqmid_getopt[] = {
	{ NULL, 0, NULL, 0 }
};
#undef __uqmi_command

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s <options|actions>\n"
		"\n", progname);
	return 1;
}

static void handle_exit_signal(int signal)
{
	uloop_end();
}

int main(int argc, char **argv)
{
	int ch, ret;

	uloop_init();
	signal(SIGINT, handle_exit_signal);
	signal(SIGTERM, handle_exit_signal);

	while ((ch = getopt_long(argc, argv, "d:", uqmid_getopt, NULL)) != -1) {
		switch(ch) {
		default:
			return usage(argv[0]);
		}
	}

	uloop_init();
	/* TODO: add option for the ubus path */
	ret = uqmid_ubus_init(NULL);
	if (ret) {
		fprintf(stderr, "Failed to connect");
		exit(1);
	}

	uloop_run();

	return ret;
}
