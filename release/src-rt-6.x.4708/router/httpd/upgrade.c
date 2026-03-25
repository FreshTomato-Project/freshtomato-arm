/*
 *
 * Tomato Firmware
 * Copyright (C) 2006-2009 Jonathan Zarate
 *
 * Fixes/updates (C) 2018 - 2026 pedro
 * https://freshtomato.org/
 *
 */


#include "tomato.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include <typedefs.h>
#include <sys/reboot.h>

/* Maximum firmware image size: 64MB. Rejects absurdly large uploads
 * before allocating memory or touching flash.
 */
#define FIRMWARE_MAX_SIZE (64 * 1024 * 1024)

void prepare_upgrade(void)
{
	int n;

	/* stop non-essential stuff & free up some memory */
	exec_service("upgrade-start");
	for (n = 60; n > 0; --n) { /* wait 60 seconds for completion */
		sleep(1);

		if (nvram_match("action_service", "")) /* this is cleared at the end */
			break;
	}

	nvram_set("os_version_last", tomato_shortver);
	nvram_commit();

	unlink("/var/log/messages");
	unlink("/var/log/messages.0");
	sync();
}

void wi_upgrade(char *url, int len, char *boundary)
{
	FILE *f = NULL;
	char fifo[] = "/tmp/flashXXXXXX";
	uint8 buf[1024];
	char *tmp;
	pid_t pid = -1;
	int fd = -1, m;
	unsigned int reset;
	const char *error = "Error reading file";
#ifdef TCONFIG_BCMARM
	char *args[] = { "mtd-write2", fifo, "linux", NULL };
#else
	char *args[] = { "mtd-write", "-w", "-i", fifo, "-d", "linux", NULL };
#endif

	check_id(url);
	reset = (strcmp(webcgi_safeget("_reset", "0"), "1") == 0);
	memset(buf, 0, sizeof(buf)); /* reset */

	/* skip the rest of the header */
	if (!skip_header(&len))
		goto ERROR;

	/* sanity check file size: must be between 1MB and FIRMWARE_MAX_SIZE */
	if (len < (1 * 1024 * 1024)) {
		error = "Invalid file: too small";
		goto ERROR;
	}
	if (len > FIRMWARE_MAX_SIZE) {
		error = "Invalid file: too large";
		goto ERROR;
	}

	if ((tmp = malloc(len)) == NULL) {
		error = "Not enough memory";
		goto ERROR;
	}
	free(tmp);

	/* -- anything after here ends in a reboot -- */

	rboot = 1;

	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	prepare_upgrade();

	/* copy web assets to /tmp for use during reboot page */
	eval("cp", "/www/reboot.asp", "/tmp");
	eval("cp", "/www/favicon.ico", "/tmp");
	eval("cp", "/www/asus-bg.png", "/tmp");
	eval("cp", "/www/tomatousb_bg.png", "/tmp");

	led(LED_DIAG, 1);

	/* mkstemp creates and opens a unique temp file; unlink it immediately
	 * so the name is free for mkfifo. fd is kept open to prevent name reuse
	 * by another process (mitigates TOCTOU race on the fifo path).
	 */
	fd = mkstemp(fifo); /* NOTE: operator precedence - assign fd first, then check */
	if (fd < 0) {
		error = "Unable to create temp file";
		goto ERROR2;
	}
	close(fd);
	fd = -1;
	unlink(fifo);

	/* create fifo at the now-free unique path */
	if (mkfifo(fifo, S_IRWXU) < 0) {
		error = "Unable to create fifo";
		goto ERROR2;
	}

	/* start mtd-write with the fifo */
	if (_eval(args, ">/tmp/.mtd-write", 0, &pid) != 0) {
		error = "Unable to start flash program";
		goto ERROR2;
	}

	/* open fifo for write */
	if ((f = fopen(fifo, "w")) == NULL) {
		error = "Unable to start pipe for mtd-write";
		goto ERROR2;
	}

	/* this will actually write the boundary, but since mtd-write uses trx length... */
	while (len > 0) {
		if ((m = web_read(buf, MIN((unsigned int)len, sizeof(buf)))) <= 0)
			goto ERROR2;

		len -= m;
		if (safe_fwrite(buf, 1, m, f) != m) {
			error = "Error writing to pipe";
			goto ERROR2;
		}
	}

	error = NULL;

ERROR2:
	if (f)
		fclose(f);

	if (fd != -1)
		close(fd);

	if (pid != -1)
		waitpid(pid, &m, 0);

	/* clear nvram? */
	if (error == NULL && reset) {
		set_action(ACT_IDLE);
#ifdef TCONFIG_BCMARM
		eval("mtd-erase2", "nvram");
#else
		eval("mtd-erase", "-d", "nvram");
#endif
	}
	set_action(ACT_REBOOT);

	/* display info on reboot page given by mtd-write (takes priority over regular error) */
	if (resmsg_fread("/tmp/.mtd-write"))
		error = NULL;

ERROR:
	/* erase flash file and free memory */
	if (fifo[0])
		unlink(fifo);

	if (error)
		resmsg_set(error);

	if (reset)
		webcgi_set("resreset", "1");

	web_eat(len);
}

void wo_flash(char *url)
{
	if (rboot) {
		sleep(1);
		parse_asp("/tmp/reboot.asp");
		web_close();

		if (nvram_get_int("remote_upgrade")) {
			killall("xl2tpd", SIGTERM);
			killall("pppd", SIGTERM);
		}

		sleep(2);

		sync();
		//kill(1, SIGTERM);
		reboot(RB_AUTOBOOT);

		exit(0);
	}

	parse_asp("error.asp");
}
