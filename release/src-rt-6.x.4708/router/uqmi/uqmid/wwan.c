/*
 * uqmid -- implement a daemon based on the uqmi idea
 *
 * Copyright (C) 2023-2024 Alexander Couzens <lynxis@fe80.eu>
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

/** Interface with the linux kernel module */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <linux/limits.h>
#include <talloc.h>

#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <linux/sockios.h>

#include "wwan.h"
#include "logging.h"
#include "modem.h"

static int get_real_device(const char *cdc_device_path, char *real_path, size_t real_path_size)
{
	ssize_t ret = readlink(cdc_device_path, real_path, real_path_size - 1);
	if (ret == -1) {
		strncpy(real_path, cdc_device_path, real_path_size - 1);
	} else if (ret == real_path_size - 1) {
		real_path[real_path_size - 1] = 0;
	}

	return 0;
}

static int get_real_device_name(const char *cdc_device_path, char *real_name, size_t real_name_size)
{
	char real_device[PATH_MAX];
	char *base;

	get_real_device(cdc_device_path, real_device, PATH_MAX);
	base = basename(real_device);
	strncpy(real_name, base, real_name_size - 1);

	return 0;
}

static int _get_wwan_device(const char *sysfs_path,
			    char *wwan_device, size_t wwan_device_size)
{
	DIR *dirfd;
	int found = 0;
	struct dirent *e;

	dirfd = opendir(sysfs_path);
	if (!dirfd) {
		return -1;
	}

	while ((e = readdir(dirfd)) != NULL)
	{
		if (!strncmp(e->d_name, ".", strlen(e->d_name)) ||
		    !strncmp(e->d_name, "..", strlen(e->d_name)))
			continue;

		if (found == 0)
			strncpy(wwan_device, e->d_name, wwan_device_size - 1);
		found++;
	}

	closedir(dirfd);
	return found;
}

/**
 *
 * @param modem
 * @param wwan_device result if found
 * @param wwan_device_size
 * @return 0 on success
 */
int wwan_refresh_device(struct modem *modem)
{
	char sysfs_path[PATH_MAX];
	char cdcname[128];
	char wwan_device[17];
	int ret;

	get_real_device_name(modem->device, cdcname, sizeof(cdcname));
	TALLOC_FREE(modem->wwan.dev);

	/*
	 * usbmisc >= 3.6
	 * usb < 3.6
	 */
	snprintf(&sysfs_path[0], sizeof(sysfs_path)-1, "/sys/class/%s/%s/device/net/", "usbmisc", cdcname);
	ret = _get_wwan_device(sysfs_path, wwan_device, sizeof(wwan_device));
	if (ret >= 1) {
		snprintf(sysfs_path, sizeof(sysfs_path)-1, "/sys/class/%s/%s/device/net/%s", "usbmisc", cdcname, wwan_device);
		modem->wwan.dev = talloc_strdup(modem, wwan_device);
		modem->wwan.sysfs = talloc_strdup(modem, sysfs_path);
		modem->subsystem_name = "usbmisc";
		return 0;
	}

	snprintf(sysfs_path, sizeof(sysfs_path)-1, "/sys/class/%s/%s/device/net/", "usb", wwan_device);
	ret = _get_wwan_device(sysfs_path, wwan_device, sizeof(wwan_device));
	if (ret >= 1) {
		snprintf(sysfs_path, sizeof(sysfs_path)-1, "/sys/class/%s/%s/device/net/%s", "usbmisc", cdcname, wwan_device);
		modem->wwan.dev = talloc_strdup(modem, wwan_device);
		modem->wwan.sysfs = talloc_strdup(modem, sysfs_path);
		modem->subsystem_name = "usb";
		return 0;
	}

	return -1;
}

/* read_uint_from_file from fstools under GPLv2 */
static int
read_char_from_file(const char *dirname, const char *filename, char *achar)
{
	FILE *f;
	char fname[PATH_MAX];
	int ret = -1;

	snprintf(fname, sizeof(fname), "%s/%s", dirname, filename);
	f = fopen(fname, "r");
	if (!f)
		return ret;

	if (fread(achar, 1, 1, f) == 1)
		ret = 0;

	fclose(f);
	return ret;
}

static int
write_char_to_file(const char *dirname, const char *filename, const char *achar)
{
	FILE *f;
	char fname[PATH_MAX];
	int ret = -1;

	snprintf(fname, sizeof(fname), "%s/%s", dirname, filename);

	f = fopen(fname, "w");
	if (!f)
		return ret;

	if (fwrite(achar, 1, 1, f) == 1)
		ret = 0;

	fclose(f);
	return ret;
}

/* use ioctl until uqmid has netlink support */
int wwan_set_mtu(const char *netif, unsigned int mtu)
{
	int sock, rc;
	struct ifreq req;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return sock;

	memset(&req, 0, sizeof req);
	strncpy(req.ifr_name, netif, sizeof(req.ifr_name));

	rc = ioctl(sock, SIOCGIFMTU, &req);
	if (rc < 0) {
		close(sock);
		return rc;
	}

	if (req.ifr_mtu == mtu) {
		close(sock);
		return 0;
	}

	req.ifr_mtu = mtu;

	rc = ioctl(sock, SIOCSIFMTU, &req);
	close(sock);
	return rc;
}

/* use ioctl until uqmid has netlink support */
int wwan_ifupdown(const char *netif, bool up)
{
	int sock, rc;
	struct ifreq req;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return sock;

	memset(&req, 0, sizeof req);
	strncpy(req.ifr_name, netif, sizeof(req.ifr_name));

	rc = ioctl(sock, SIOCGIFFLAGS, &req);
	if (rc < 0) {
		close(sock);
		return rc;
	}

	if ((req.ifr_flags & IFF_UP) == up) {
		close(sock);
		return 0;
	}

	if (up)
		req.ifr_flags |= IFF_UP;
	else
		req.ifr_flags &= ~IFF_UP;

	rc = ioctl(sock, SIOCSIFFLAGS, &req);
	close(sock);
	return rc;
}

int wwan_read_configuration(const char *sysfs_path, struct wwan_conf *config)
{
	char tmp = 0;
	int ret;
	char qmi_path[PATH_MAX];

	snprintf(qmi_path, sizeof(qmi_path) - 1, "%s/qmi", sysfs_path);
	ret = read_char_from_file(qmi_path, "raw_ip", &tmp);
	if (ret)
		return -ENOENT;
	config->raw_ip = (tmp == 'Y');

	ret = read_char_from_file(qmi_path, "pass_through", &tmp);
	if (ret)
		return -ENOENT;

	config->pass_through = (tmp == 'Y');

	return 0;
}

/**
 *
 * @param sysfs_path path to the network sysfs path
 * @param config
 * @return 0 on success
 */
int
wwan_set_configuration(const char *sysfs_path, const struct wwan_conf *config)
{
	struct wwan_conf old = { 0 };
	int ret;
	char yes = 'Y';
	char no = 'N';
	char qmi_path[PATH_MAX];

	snprintf(qmi_path, sizeof(qmi_path) - 1, "%s/qmi", sysfs_path);
	ret = wwan_read_configuration(sysfs_path, &old);
	if (ret) {
		return -ENOENT;
	}

	if (config->raw_ip != old.raw_ip) {
		ret = write_char_to_file(qmi_path, "raw_ip", config->raw_ip ? &yes : &no);
		if (ret) {
			return -EINVAL;
		}
	}

	if (config->pass_through != old.pass_through) {
		ret = write_char_to_file(qmi_path, "pass_through", config->pass_through ? &yes : &no);
		if (ret) {
			return -EINVAL;
		}
	}

	return 0;
}
