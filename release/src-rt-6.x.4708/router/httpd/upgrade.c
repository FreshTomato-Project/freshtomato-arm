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
#include <typedefs.h>
#include <sys/reboot.h>

/* Maximum firmware image size: 64MB. Rejects absurdly large uploads
 * before allocating memory or touching flash.
 */
#define FIRMWARE_MAX_SIZE	(64 * 1024 * 1024)
#define FIRMWARE_TMP_RESERVE	(1 * 1024 * 1024)

/* needed by logmsg() */
#define LOGMSG_DISABLE		DISABLE_SYSLOG_OSM
#define LOGMSG_NVDEBUG		"upgrade_debug"

static char upgrade_file[64];
static unsigned int upgrade_reset;


static int wait_upgrade_service(const char *action, int timeout)
{
	exec_service(action);

	while (timeout-- > 0) {
		if (nvram_match("action_service", ""))
			return 1;

		sleep(1);
	}

	return nvram_match("action_service", "");
}

static int firmware_tmp_space_ok(unsigned long image_len)
{
	struct statfs fs;
	unsigned long long available;
	unsigned long long needed;

	if (statfs("/tmp", &fs) != 0)
		return 0;

	available = (unsigned long long)fs.f_bavail * (unsigned long long)fs.f_bsize;
	needed = (unsigned long long)image_len + FIRMWARE_TMP_RESERVE;

	return (available >= needed);
}

static int validate_firmware(const char *file)
{
	int status;

#ifdef TCONFIG_BCMARM
	char *args[] = { "mtd-write2", "-c", (char *)file, "linux", NULL };
#else
	char *args[] = { "mtd-write", "-c", "-i", (char *)file, "-d", "linux", NULL };
#endif

	unlink("/tmp/.mtd-check");
	status = _eval(args, ">/tmp/.mtd-check", 0, NULL);

	return status;
}

int prepare_upgrade(void)
{
	int clean;

	/* stop non-essential stuff & free up some memory */
	clean = wait_upgrade_service("upgrade-start", 60);
	if (!clean)
		logmsg(LOG_WARNING, "upgrade-start did not complete before timeout; continuing");

	nvram_set("os_version_last", tomato_shortver);
	nvram_commit();

	sync();

	return clean;
}

int finalize_upgrade(void)
{
	/*
	 * Stop the listening httpd master without killing this request worker:
	 * it still has to run the validated MTD write after finalization.
	 * Drop the worker's inherited web_dir cwd as well so /opt or another
	 * USB-backed web root cannot keep storage busy during unmount.
	 */
	kill_pidfile_s("/var/run/httpd.pid", SIGTERM);
	chdir("/");

	return wait_upgrade_service("upgradefinalize-start", 60);
}

void wi_upgrade(char *url, int len, char *boundary)
{
	FILE *f = NULL;
	struct stat st;
	char end_boundary[256];
	char recv_boundary[256];
	char tail[2];
	uint8 buf[4096];
	unsigned long image_len;
	unsigned long remaining;
	int m;
	int fd = -1;
	int end_len;
	int status;
	unsigned int reset;
	const char *error = "Error reading file";

	upgrade_file[0] = '\0';
	upgrade_reset = 0;
	rboot = 0;

	/* validate session */
	check_id(url);

	reset = (strcmp(webcgi_safeget("_reset", "0"), "1") == 0);

	/* Skip multipart headers and leave len at the firmware payload. */
	if (!skip_header(&len))
		goto ERROR;

	if ((boundary == NULL) || (*boundary == '\0')) {
		error = "Invalid upload boundary";
		goto ERROR;
	}

	end_len = snprintf(end_boundary, sizeof(end_boundary), "\r\n--%s--", boundary);
	if ((end_len <= 0) || (end_len >= (int)sizeof(end_boundary)) || (len <= (end_len + 2))) {
		error = "Invalid upload boundary";
		goto ERROR;
	}

	/*
	 * The multipart body ends with:
	 *   firmware data + "\\r\\n--" + boundary + "--\\r\\n"
	 * Stage only the firmware bytes so the validator sees the exact image.
	 */
	image_len = (unsigned long)(len - end_len - 2);

	if (image_len < (1 * 1024 * 1024)) {
		error = "Invalid file: too small";
		goto ERROR;
	}
	if (image_len > FIRMWARE_MAX_SIZE) {
		error = "Invalid file: too large";
		goto ERROR;
	}

	if (!firmware_tmp_space_ok(image_len)) {
		error = "Not enough free memory to stage firmware image";
		goto ERROR;
	}

	strlcpy(upgrade_file, "/tmp/firmwareXXXXXX", sizeof(upgrade_file));
	if ((fd = mkstemp(upgrade_file)) < 0) {
		error = "Unable to create temporary firmware file";
		goto ERROR;
	}

	if ((f = fdopen(fd, "w")) == NULL) {
		error = "Unable to open temporary firmware file";
		goto ERROR;
	}
	fd = -1; /* owned by f */

	remaining = image_len;
	while (remaining > 0) {
		m = web_read(buf, MIN(remaining, (unsigned long)sizeof(buf)));
		if (m <= 0) {
			error = "Incomplete firmware upload";
			goto ERROR;
		}

		if (safe_fwrite(buf, 1, m, f) != (size_t)m) {
			error = "Error writing temporary firmware file";
			goto ERROR;
		}

		remaining -= (unsigned long)m;
		len -= m;
	}

	if ((fflush(f) != 0) || (fsync(fileno(f)) != 0)) {
		error = "Error flushing temporary firmware file";
		goto ERROR;
	}

	if (fclose(f) != 0) {
		f = NULL;
		error = "Error closing temporary firmware file";
		goto ERROR;
	}
	f = NULL;

	/* Consume and verify the multipart terminator separately from the image. */
	if (web_read_x(recv_boundary, end_len) != end_len) {
		error = "Incomplete upload boundary";
		goto ERROR;
	}
	len -= end_len;

	if (memcmp(recv_boundary, end_boundary, end_len) != 0) {
		error = "Invalid upload boundary";
		goto ERROR;
	}

	if (web_read_x(tail, (int)sizeof(tail)) != (int)sizeof(tail)) {
		error = "Incomplete upload boundary";
		goto ERROR;
	}
	len -= (int)sizeof(tail);

	if ((tail[0] != '\r') || (tail[1] != '\n')) {
		error = "Invalid upload boundary";
		goto ERROR;
	}

	if ((stat(upgrade_file, &st) != 0) || ((unsigned long)st.st_size != image_len)) {
		error = "Incomplete firmware upload";
		goto ERROR;
	}

	/*
	 * Validate while the router is still fully operational. A bad header,
	 * size or CRC is reported to the browser without touching flash or
	 * stopping services.
	 */
	status = validate_firmware(upgrade_file);
	if (status != 0) {
		if (!resmsg_fread("/tmp/.mtd-check"))
			error = "Firmware image validation failed";
		else
			error = NULL;
		goto ERROR;
	}
	unlink("/tmp/.mtd-check");

	/* From this point forward the validated image is committed for upgrade. */
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (!prepare_upgrade())
		resmsg_set("Warning: service shutdown timed out after 60 seconds; firmware upgrade will continue. If this repeats, check enabled service configuration.");

	upgrade_reset = reset;
	rboot = 1;
	led(LED_DIAG, 1);

	if (reset)
		webcgi_set("resreset", "1");

	return;

ERROR:
	if (f)
		fclose(f);
	else if (fd >= 0)
		close(fd);

	if (!rboot && upgrade_file[0]) {
		unlink(upgrade_file);
		upgrade_file[0] = '\0';
	}
	unlink("/tmp/.mtd-check");

	if (error)
		resmsg_set(error);

	/* consume any remaining unread POST data */
	if (len > 0)
		web_eat(len);
}

void wo_flash(char *url)
{
	int status;

#ifdef TCONFIG_BCMARM
	char *args[] = { "mtd-write2", upgrade_file, "linux", NULL, NULL };
#else
	char *args[] = { "mtd-write", "-w", "-i", upgrade_file, "-d", "linux", NULL };
#endif

	if (!rboot || !upgrade_file[0]) {
		parse_asp("error.asp");
		return;
	}

	/*
	 * Deliver reboot.asp while the client-facing network is still available.
	 * Keep httpd/networking alive briefly so the browser can fetch the linked
	 * stylesheets before upgrade-finalize tears the interface down.
	 */
	parse_asp("reboot.asp");
	web_close();
	sleep(2);

	/*
	 * The destructive phase starts only now. This stops wireless and any
	 * networking/storage kept alive for the upload response.
	 */
	if (!finalize_upgrade())
		logmsg(LOG_WARNING, "upgrade-finalize did not complete before timeout; continuing");

#ifdef TCONFIG_BCMARM
	/*
	 * Let the running firmware writer erase NVRAM after a successful write.
	 * Avoid exec'ing mtd-erase2 from the rootfs we have just overwritten.
	 */
	if (upgrade_reset) {
		args[3] = args[2];
		args[2] = args[1];
		args[1] = "-e";
	}
#endif

	sync();
	unlink("/tmp/.mtd-write");
	status = _eval(args, ">/tmp/.mtd-write", 0, NULL);
	if (status != 0)
		fprintf(stderr, "Firmware upgrade operation failed (status %d)\n", status);

#ifndef TCONFIG_BCMARM
	if ((status == 0) && upgrade_reset) {
		set_action(ACT_IDLE);
		eval("mtd-erase", "-d", "nvram");
	}
#endif

	unlink(upgrade_file);
	upgrade_file[0] = '\0';

	/*
	 * Once flashing has started the old root filesystem can no longer be
	 * trusted. Reboot even if the writer reports an I/O error, matching the
	 * historical web-upgrade recovery behaviour.
	 */
	set_action(ACT_REBOOT);
	sync();
	sleep(2);
	reboot(RB_AUTOBOOT);
	exit(0);
}
