# FreshTomato ARM

> Alternative open-source firmware for Broadcom ARM-based routers.

## Supported routers

| Manufacturer | Models |
|---|---|
| **ASUS** | N18U, AC56S, AC56U/AC56R, N66U C1, AC66U B1, RT-AC1750 B1, AC67U, AC68U (A1, A2, B1, B2, C1, E1, V3)/R/P, AC1900P/U, AC3200, AC3100, AC88U, AC5300, DSL-AC68U *(no xDSL support)* |
| **Netgear** | AC1450, R6200v2, R6250, R6300v2, R6400, R6400v2, R6700v1, R6700v3, R6900, XR300, R7000, EX6200, EX7000, R7900, R8000 |
| **Linksys** | EA6200, EA6350v1, EA6350v2, EA6300v1/EA6400, EA6500v2, EA6700, EA6900 |
| **Tenda** | AC15, AC18 |
| **Huawei** | WS880 |
| **D-Link** | DIR-868L (rev. A1/B1/C1) |
| **Xiaomi** | R1D |
| **Belkin** | F9K1113v2 |
| **Buffalo** | WZR-1750DHP |

> [!CAUTION]
> Flashing alternative firmware can permanently damage your router. The author is not responsible for bricked devices. Proceed entirely at your own risk.

## Project resources

| Resource | Link |
|---|---|
| Project website | [freshtomato.org](https://freshtomato.org/) |
| Source code | [GitHub](https://github.com/FreshTomato-Project/freshtomato-arm) · [GitLab mirror](https://gitlab.com/pedro311/freshtomato-arm) |
| Changelog | [CHANGELOG](https://github.com/FreshTomato-Project/freshtomato-arm/blob/arm-master/CHANGELOG) |
| Downloads | [Download images](https://freshtomato.org/downloads) |
| Issue tracker | [GitHub Issues](https://github.com/FreshTomato-Project/freshtomato-arm/issues) |
| Pull requests | [GitHub Pull Requests](https://github.com/FreshTomato-Project/freshtomato-arm/pulls) |
| English forum | [LinksysInfo](https://www.linksysinfo.org/index.php?forums/tomato-firmware.33/) |
| Polish forum | [OpenLinksys](https://openlinksys.info/forum/) |
| Donations | [Support FreshTomato](https://freshtomato.org/donations.html) |

## Preparing the build environment

The following instructions target **Debian 13 (64-bit)**.

### 1. Install Debian

Install Debian using the graphical installer. For simplicity:

- enable the SSH server;
- create a standard user account;
- keep the remaining options at their defaults.

In the commands below, replace `<username>`, `<name>`, and `<email-address>` with your own values.

### 2. Update the system

Log in as `root`, then run:

```sh
apt-get update
apt-get dist-upgrade
```

### 3. Install basic packages

```sh
apt-get install build-essential net-tools
```

### 4. Configure the time zone

```sh
dpkg-reconfigure tzdata
```

If the command is unavailable because of the current `PATH`, run:

```sh
export PATH="$PATH:/usr/sbin"
```

### 5. Grant sudo access

```sh
apt-get install sudo
adduser <username> sudo
reboot
```

After the reboot, log in as `<username>`.

### 6. Install build dependencies

```sh
sudo apt-get install \
  autoconf autoconf-archive m4 bison flex g++ libtool gcc binutils patch \
  bzip2 make gettext unzip zlib1g-dev libc6 gperf automake groff minisign

sudo apt-get install \
  lib32stdc++6 libncurses6 libncurses-dev gawk gitk zlib1g-dev autopoint \
  shtool autogen mtd-utils gcc-multilib lib32z1-dev pkg-config libssl-dev \
  automake

sudo apt-get install \
  libmnl-dev libxml2-dev intltool libglib2.0-dev texinfo dos2unix xsltproc \
  libnfnetlink0 libcurl4-openssl-dev libgtk2.0-dev libnotify-dev \
  libevent-dev git

sudo apt-get install \
  re2c texlive libelf1 nodejs zip mc cmake ninja-build curl \
  libglib2.0-dev-bin libglib2.0-dev sqlite3 dconf-editor python3-dev \
  python3-setuptools

sudo apt-get install "linux-headers-$(uname -r)"
```

### 7. Clone the repository

```sh
git clone https://github.com/FreshTomato-Project/freshtomato-arm.git
```

### 8. Reboot

```sh
sudo reboot
```

### 9. Configure Git

```sh
cd freshtomato-arm
git config --global user.email "<email-address>"
git config --global user.name "<name>"
```

## Compiling FreshTomato

### Prepare the repository

Enter the repository:

```sh
cd freshtomato-arm
```

Before each compilation, clean and reset the working tree:

```sh
git clean -fdxq
git reset --hard
```

Optionally, download the latest changes:

```sh
git pull
```

> [!WARNING]
> `git clean -fdxq` and `git reset --hard` permanently remove untracked files and local changes.

### SDK 6

Example: build an AIO image for **RT-N18U/AC56S without SMP**.

```sh
git checkout arm-master
cd release/src-rt-6.x.4708
make help
make n18z
```

### SDK 7

Example: build an AIO image for **RT-AC3200**.

```sh
git checkout arm-master
cd release/src-rt-7.x.main/src
make help
make ac3200-128z
```

### SDK 7.14

Example: build an AIO image for **RT-AC5300**.

```sh
git checkout arm-master
cd release/src-rt-7.14.114.x/src
make help
make ac5300-128z
```

`make help` lists the build targets available in the selected source tree.
