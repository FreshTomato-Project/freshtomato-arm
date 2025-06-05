# **FreshTomato-ARM** #
  
  
**Alternative open source firmware for Broadcom ARM based routers**
  
  
For the following **ARM** routers: **Asus** N18U, AC56S, AC56U/AC56R, N66U C1, AC66U B1, RT-AC1750 B1, AC67U, AC68U(A1,A2,B1,B2,C1,E1,V3)/R/P, AC1900P/U, AC3200, AC3100, AC88U, AC5300, DSL-AC68U(no xDSL support) **Netgear** AC1450, R6200v2, R6250, R6300v2, R6400, R6400v2, R6700v1, R6700v3, R6900, XR300, R7000, EX6200, EX7000, R7900, R8000, **Linksys** EA6200, EA6350v1, EA6350v2, EA6300v1/EA6400, EA6500v2, EA6700, EA6900, **Tenda** AC15, AC18, **Huawei** WS880, **Dlink** DIR868L (rev A1/B1/C1), **Xiaomi** R1D, **Belkin** F9K1113v2, **Buffalo** WZR-1750DHP.  
  
  
***Disclaimer: I am not responsible for any bricked routers, nor do I encourage other people to flash alternative firmwares on their routers. Use at your own risk!***  
  
  
- [**Project page**](https://freshtomato.org/)
- [**Source code**](https://github.com/FreshTomato-Project/freshtomato-arm) ([**Mirror**](https://gitlab.com/pedro311/freshtomato-arm))
- [**Changelog**](https://github.com/FreshTomato-Project/freshtomato-arm/blob/arm-master/CHANGELOG)
- [**Downloads**](https://freshtomato.org/downloads)
- [**Issue tracker**](https://github.com/FreshTomato-Project/freshtomato-arm/issues)
- [**Pull Requests**](https://github.com/FreshTomato-Project/freshtomato-arm/pulls)
- [**Forum EN**](https://www.linksysinfo.org/index.php?forums/tomato-firmware.33/)
- [**Forum PL**](https://openlinksys.info/forum/)
- **Donations**: [**PayPal**](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=B4FDH9TH6Z8FU)  or  BTC: **`1JDxBBQvcJ9XxgagJRNVrqC1nysq8F8B1Y`**  
  
  
**HOW TO PREPARE A WORK ENVIRONMENT FOR FRESHTOMATO COMPILATION (on Debian 12/64bit)**
  
1. Install Debian via the graphical interface (for simplicity); install the SSH server, choose default [username]; the rest may be the default
  
2. Login as root
  
3. Update system:
    ```sh
    apt-get update
    apt-get dist-upgrade
    ```
  
4. Install basic packages:
    ```sh
    apt-get install build-essential net-tools
    ```
  
5. Set proper date/time:
    ```sh
    dpkg-reconfigure tzdata
    ```
    In case of problems here:
    ```sh
    export PATH=$PATH:/usr/sbin
    ```
  
6. Add your [username] to sudo group:
    ```sh
    apt-get install sudo
    adduser [username] sudo
    reboot
    ```
  
7. Login as [username], install base packages with all dependencies:
    ```sh
    sudo apt-get install autoconf autoconf-archive m4 bison flex g++ libtool gcc binutils patch bzip2 make gettext unzip zlib1g-dev libc6 gperf automake groff
    sudo apt-get install lib32stdc++6 libncurses5 libncurses5-dev gawk gitk zlib1g-dev autopoint shtool autogen mtd-utils gcc-multilib lib32z1-dev pkg-config libssl-dev automake1.11
    sudo apt-get install libmnl-dev libxml2-dev intltool libglib2.0-dev libstdc++5 texinfo dos2unix xsltproc libnfnetlink0 libcurl4-openssl-dev libgtk2.0-dev libnotify-dev libevent-dev git
    sudo apt-get install re2c texlive libelf1 nodejs zip mc cmake ninja-build curl libglib2.0-dev-bin libglib2.0-dev sqlite3 dconf-editor python3.11-dev
    sudo apt-get install linux-headers-$(uname -r)
    ```
  
8. Clone/download repository:
    ```sh
    git clone https://github.com/FreshTomato-Project/freshtomato-arm.git
    ```
  
9. Reboot system
  
10. Add your email address to git config:
    ```sh
    cd freshtomato-arm
    git config --global user.email "[email-address]"
    ```
  
11. Add your username to git config:
    ```sh
    cd freshtomato-arm
    git config --global user.name [name]
    ```
  
  
**HOW TO COMPILE**
  
1. Change dir to git repository ie: ```cd freshtomato-arm```
2. Before every compilation, use ```git clean -fdxq && git reset --hard```, and possibly ```git pull``` to pull recent changes from remote
3. To compile SDK6 image, use: ```git checkout arm-master``` then: ```cd release/src-rt-6.x.4708```, check for possible targets: ```make help```, use one (RT-N18U/AC56S without SMP build AIO): ```make n18z```
4. To compile SDK7 image, use: ```git checkout arm-master``` then: ```cd release/src-rt-7.x.main/src```, check for possible targets: ```make help```, use one (RT-AC3200 build AIO): ```make ac3200-128z```
5. To compile SDK714 image, use: ```git checkout arm-master``` then: ```cd release/src-rt-7.14.114.x/src```, check for possible targets: ```make help```, use one (RT-AC5300 build AIO): ```make ac5300-128z```
  
