# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# To rebuild everything and all configurations:
#  make distclean
#  make V1=whatever V2=sub-whatever VPN=vpn3.6 X
#
# The 1st "whatever" would be the build number - use only when you want the build number to be added 
# to the tomato version; the sub-whatever would be the version note (both optional).
#
# Example:
# make V1=8516 V2="my_build" X
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


export ac_cv_func_malloc_0_nonnull=yes
export SRCBASE := $(shell pwd)
export SRCBASEDIR := $(shell pwd | sed 's/.*release\///g')
RELEASEDIR := $(shell (cd $(SRCBASE)/.. && pwd -P))
PATH := $(RELEASEDIR)/tools:$(PATH)

include target.mak

ifeq ($(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73)$(CONFIG_TOOLCHAIN75),)
 export PATH := $(PATH):$(PWD)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin
 export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3/lib
else
 ifeq ($(CONFIG_TOOLCHAIN73),y)
  export PATH := $(PATH):$(PWD)/toolchains/hndtools-arm-uclibc-7.3/usr/bin
  export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-uclibc-7.3/usr/lib
 else
  ifeq ($(CONFIG_TOOLCHAIN75),y)
   export PATH := $(PATH):$(PWD)/toolchains/hndtools-arm-uclibc-7.5/usr/bin
   export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-uclibc-7.5/usr/lib
  else
   export PATH := $(PATH):$(PWD)/toolchains/hndtools-arm-uclibc-5.3/usr/bin
   export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-uclibc-5.3/usr/lib
  endif
 endif
endif

CTAGS_EXCLUDE_OPT := --exclude=kernel_header --exclude=$(PLATFORM)
CTAGS_DEFAULT_DIRS := $(SRCBASE)/router/rc $(SRCBASE)/router/httpd $(SRCBASE)/src/router/shared $(SRCBASE)/src/router/www
KERNEL_BINARY = $(LINUXDIR)/arch/arm/boot/zImage

export TPROFILE := N
V1 ?= "--def"
VPN ?= "VPN"
PPTPD ?= "n"
KERN_SIZE_OPT ?= n

# Asus firmware version in the TRX header (prevents downgrading to an incompatible firmware at AsusWRT)
FORCE_SN = 386
FORCE_EN = 40000

ifeq ($(NVRAM_SIZE),)
 NVRAM_SIZE = 0
endif

ifeq ($(ASUS_TRX),)
 ASUS_TRX = 0
endif

ifeq ($(NETGEAR_CHK),)
 NETGEAR_CHK = 0
else
 WNRTOOL=$(SRCBASE)/wnrtool
 BOARD_FILE=$(WNRTOOL)/$(BOARDID_FILE)
 FW_FILE=$(WNRTOOL)/$(FW_CFG_FILE)
endif

ifeq ($(XIAOMI_TRX),)
 XIAOMI_TRX = 0
endif

ifeq ($(NVRAM_64K),y)
 EXTRA_64KDESC = " -64K"
 EXTRA_64KCFLAG = -DTCONFIG_NVRAM_64K
else
 ifeq ($(NVRAM_128K),y)
  EXTRA_64KCFLAG = -DTCONFIG_NVRAM_128K
  ifeq ($(DRAM_512M),y)
   EXTRA_64KDESC = "-128K-512M"
  else
   EXTRA_64KDESC = "-128K"
   ifneq ($(NVSIZE),)
    EXTRA_64KCFLAG += -DCONFIG_NVSIZE_128
   endif
  endif
 else
  EXTRA_64KDESC =
  EXTRA_64KCFLAG =
 endif
endif

NOSMP_DESC =
ifeq ($(ASUS_TRX),ASUS)
 ifneq ($(BCMSMP),y)
  NOSMP_DESC = "-NOSMP"
 endif
endif
ifeq ($(ASUS_TRX),LINKSYS2)
 ifneq ($(BCMSMP),y)
  NOSMP_DESC = "-NOSMP"
 endif
endif
ifeq ($(ASUS_TRX),BELKIN)
 ifneq ($(BCMSMP),y)
  NOSMP_DESC = "-NOSMP"
 endif
endif
ifeq ($(ASUS_TRX),NETGEAR2)
 ifneq ($(BCMSMP),y)
  NOSMP_DESC = "-NOSMP"
 endif
endif

SDK7WL_DESC =
ifeq ($(BCM7WL2020),y)
 SDK7WL_DESC = "-WL20"
endif
ifeq ($(BCM7WL2021),y)
 SDK7WL_DESC = "-WL21"
endif

-include tomato_profile.mak

# This could be simpler by just using $(TOMATO_PROFILE_NAME) like it used to be,
# but that's fragile.  If you make one certain innocuous change elsewhere in the
# makefile(s), the build will silently be wrong. This way it works properly every time.
current_BUILD_NAME = $(strip $(shell grep "^TOMATO_BUILD_NAME" tomato_profile.mak | cut -d"=" -f2))
current_BUILD_DESC = $(strip $(shell grep "^TOMATO_BUILD_DESC" tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_BUILD_USB  = $(strip $(shell grep "^TOMATO_BUILD_USB"  tomato_profile.mak | cut -d"=" -f2 | sed -e "s/ //g"))
current_TOMATO_VER = $(strip $(shell grep "TOMATO_MAJOR" router/shared/tomato_version.h | cut -d"\"" -f2)).$(strip $(shell grep "TOMATO_MINOR" router/shared/tomato_version.h | cut -d"\"" -f2))$(if $(filter $(V1),"--def"),,.$(strip $(shell grep -w "TOMATO_BUILD" router/shared/tomato_version.h | cut -d"\"" -f2)))

uppercase_N = $(shell echo $(N) | tr a-z A-Z)
lowercase_N = $(shell echo $(N) | tr A-Z a-z)
uppercase_B = $(shell echo $(B) | tr a-z A-Z)
lowercase_B = $(shell echo $(B) | tr A-Z a-z)

beta = $(if $(filter $(TOMATO_EXPERIMENTAL),1),-beta,)

ifeq ($(TARGET_VARIANT),SDK6)
 branch_rev = "K26ARM"
else ifeq ($(TARGET_VARIANT),SDK7)
 branch_rev = "K26ARM7"
else ifeq ($(TARGET_VARIANT),SDK714)
 branch_rev = "K26ARM714"
else
 $(error Unsupported TARGET_VARIANT='$(TARGET_VARIANT)' for TARGET_PLATFORM='$(TARGET_PLATFORM)')
endif

ifneq ($(V2),)
	current_V2 = $(shell echo -$(V2))
else
	current_V2 = ""
endif

# Macro to inject firmware name and package the firmware (Default: Asus Header)
define CREATE_INJECT_MODEL
	# 1. Prepare environment: Create target directory and remove old artifacts
	mkdir -p router/arm-uclibc/target/rom/filename
	rm -rf router/arm-uclibc/target/rom/filename/*
	rm -f image/linux-lzma.trx

	# 2. Sanitize the model name: Remove quotes and store in a variable
	$(eval SAN_IMAGE := $(shell echo $(1) | tr -d '"' | tr -d "'"))

	# 3. Create a placeholder file with the sanitized name inside the target folder
	printf "" > router/arm-uclibc/target/rom/filename/$(SAN_IMAGE)

	# 4. Generate the SquashFS filesystem from the target directory
	ctools/mksquashfs router/arm-uclibc/target ctools/target.squashfs -noappend -all-root

	# 5. Build the standard TRX image
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs

	# 6. Add ASUS-specific headers and create the final flashable image
	ctools/trx_asus -i image/linux-lzma.trx -r $(2),$(3),$(4),$(5),image/$(SAN_IMAGE)
endef

define CREATE_INJECT_MODEL_STD
	# 1. Prepare environment: Create target directory and remove old artifacts
	mkdir -p router/arm-uclibc/target/rom/filename
	rm -rf router/arm-uclibc/target/rom/filename/*
	rm -f image/linux-lzma.trx

	# 2. Sanitize the model name: Remove quotes and store in a variable
	$(eval SAN_IMAGE := $(shell echo $(1) | tr -d '"' | tr -d "'"))

	# 3. Create a placeholder file with the sanitized name inside the target folder
	printf "" > router/arm-uclibc/target/rom/filename/$(SAN_IMAGE)

	# 4. Generate the SquashFS filesystem from the target directory
	ctools/mksquashfs router/arm-uclibc/target ctools/target.squashfs -noappend -all-root

	# 5. Build the standard TRX image
	ctools/trx -o image/linux-lzma.trx ctools/vmlinuz-lzma ctools/target.squashfs
endef

tomato_ver:
	@echo ""
	@btools/uversion.pl --gen $(V1) $(branch_rev) $(beta) $(V2) $(current_BUILD_USB) $(current_BUILD_DESC)

include build.mak

all: tomato_ver build_all
	@echo ""
	@echo "-----------------"
	@echo `cat router/shared/tomato_version` " ready"
	@echo "-----------------"
	@echo "\033[41;1m   FreshTomato build done! \033[0m\033]2;FreshTomato build done!\007"

clean:
	@touch router/.config
	@rm -f router/config_[a-z]
	@rm -f router/busybox/config_[a-z]
	@$(MAKE) -C router $@
	@-$(MAKE) cleantools
	@-rmdir router/arm-uclibc

cleanimage:
	@rm -f fpkg.log
	@rm -fr image/*
	@rm -f router/.config
	@touch router/.config 
	@-mkdir image

cleantools:
	@$(MAKE) -C btools clean
	@$(MAKE) -C ctools clean

cleankernel:
	@cd $(LINUXDIR) && \
	mv .config save-config && \
	$(MAKE) distclean || true; \
	cp -p save-config .config || true

kernel:
	@$(MAKE) -C router kernel
	@[ ! -e $(KERNEL_BINARY) ] || ls -l $(KERNEL_BINARY)

distclean: clean cleanimage cleankernel cleantools
ifneq ($(INSIDE_MAK),1)
	@$(MAKE) -C router $@ INSIDE_MAK=1
endif
	mv router/busybox/.config busybox-saved-config || true
	@$(MAKE) -C router/busybox distclean
	@rm -f router/busybox/config_current
	@cp -p busybox-saved-config router/busybox/.config || true
	@cp -p router/busybox/.config  router/busybox/config_current || true
	@rm -f router/config_current
	@rm -f router/.config.cmd router/.config.old router/.config
	@rm -f router/libfoo_xref.txt
	@rm -f tomato_profile.mak router/shared/tomato_profile.h
	@touch tomato_profile.mak
	@touch router/shared/tomato_profile.h

prepk:
	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_base config_current ; \
		cp -f config_current .config
	$(MAKE) -C $(LINUXDIR) oldconfig

what:
	@echo ""
	@echo "$(current_BUILD_DESC)-$(current_BUILD_NAME)-$(TOMATO_PROFILE_NAME) Profile"
	@echo ""

# The methodology for making the different builds is to
# copy the "base" config file to the "target" config file and
# replace boolean TCONFIG values in place. Every TCONFIG symbol
# changed by RouterOptions must already exist in config_base as
# either "TCONFIG_x=y" or "# TCONFIG_x is not set".

# Common options for "make bin":
# BUILD_DESC (Std|Lite|Ext|...)
# KERN_SIZE_OPT
# USB ("USB"|"")
# JFFSv1 | NO_JFFS
# NO_CIFS NO_SAMBA NO_HTTPS NO_ADBLOCK NO_DMZ NO_DMZMAC NO_XXTP NO_L7 NO_L7PAT NO_LIBOPT NO_FTP NO_USBAPP NO_KEYGEN
# FTPSSL OPENVPN IPV6SUPP EBTABLES NTFS UFSD MEDIASRV BBEXTRAS USBEXTRAS BCM57 SLIM NOCAT NGINX NANO PROXY
# NFS BTCLIENT BTGUI TR_EXTRAS SNMP HFS UPS DNSCRYPT DNSSEC STUBBY PPTPD TOR TINC IPSEC RAID MICROSD USBAP IPERF
# KEYGEN HAVEGED CTF BRCM_NAND_JFFS2 MDNS ROAM OPENSSL11 OPENSSL30 WOLFSSL WOLFSSLMIN QRCODE AIO MWAN_MAX BRIDGE_COUNT
# NAND NVRAM_64K NVRAM_SIZE
# ARM-specific options:
# ARM BCM7 BCM714 AC3200 AC5300 BCMSMP BSD BCMFA PROXYSTA DPSTA GRO EXTSW GMAC3 FANCTRL IRQBALANCE
# WIREGUARD DO_OPTIMIZE_SIZE NO_BONDING NO_TRXKEY NOWL NVRAM_128K CONNTRACK_TOOL CRASH_LOG TUXERA TUXERA_HFS ZFS DRAM_512M

define RouterOptions
	@( \
	config_y() { \
		grep -q -e "^$$1=y$$" -e "^# $$1 is not set$$" $(1) || { echo "ERROR: $$1 missing from $(1)" >&2; exit 1; }; \
		sed -i "s/^# $$1 is not set$$/$$1=y/" $(1); \
	}; \
	config_n() { \
		grep -q -e "^$$1=y$$" -e "^# $$1 is not set$$" $(1) || { echo "ERROR: $$1 missing from $(1)" >&2; exit 1; }; \
		sed -i "s/^$$1=y$$/# $$1 is not set/" $(1); \
	}; \
	config_value() { \
		grep -q "^$$1=" $(1) || { echo "ERROR: $$1 missing from $(1)" >&2; exit 1; }; \
		sed -i "s/^$$1=.*/$$1=$$2/" $(1); \
	}; \
	config_range() { \
		case "$$2" in ''|*[!0-9]*) echo "ERROR: $$1 must be an integer in range $$3..$$4" >&2; exit 1;; esac; \
		if [ "$$2" -lt "$$3" ] || [ "$$2" -gt "$$4" ]; then echo "ERROR: $$1=$$2 outside supported range $$3..$$4" >&2; exit 1; fi; \
		config_value "$$1" "$$2"; \
	}; \
	if [ -n "$(MWAN_MAX)" ]; then \
		config_range TCONFIG_MWAN_MAX "$(MWAN_MAX)" 1 8; \
	elif [ "$(TARGET_PLATFORM)" = "MIPS" ]; then \
		if [ "$(NVRAM_64K)" = "y" ] || [ "$(NVRAM_SIZE)" = "60" ] || [ "$(NVRAM_SIZE)" = "64" ]; then \
			config_range TCONFIG_MWAN_MAX "4" 1 8; \
		else \
			config_range TCONFIG_MWAN_MAX "2" 1 8; \
		fi; \
	elif [ "$(TARGET_PLATFORM)" = "ARM" ]; then \
		if [ "$(NVRAM_64K)" = "y" ] || [ "$(NVRAM_128K)" = "y" ] || [ "$(NVRAM_SIZE)" = "60" ] || [ "$(NVRAM_SIZE)" = "64" ]; then \
			config_range TCONFIG_MWAN_MAX "4" 1 8; \
		else \
			config_range TCONFIG_MWAN_MAX "2" 1 8; \
		fi; \
	fi; \
	if [ -n "$(BRIDGE_COUNT)" ]; then \
		config_range TCONFIG_BRIDGE_COUNT "$(BRIDGE_COUNT)" 1 16; \
	fi; \
	config_n TCONFIG_BCMARM; \
	config_n TCONFIG_BCM7; \
	config_n TCONFIG_BCM714; \
	config_n TCONFIG_MIPS_RT; \
	config_n TCONFIG_MIPS_RTN; \
	config_n TCONFIG_MIPS_RTAC; \
	case "$(TARGET_PLATFORM):$(TARGET_VARIANT)" in \
		ARM:SDK6) config_y TCONFIG_BCMARM ;; \
		ARM:SDK7) config_y TCONFIG_BCMARM; config_y TCONFIG_BCM7 ;; \
		ARM:SDK714) config_y TCONFIG_BCMARM; config_y TCONFIG_BCM7; config_y TCONFIG_BCM714 ;; \
		MIPS:RT) config_y TCONFIG_MIPS_RT ;; \
		MIPS:RTN) config_y TCONFIG_MIPS_RTN ;; \
		MIPS:RTAC) config_y TCONFIG_MIPS_RTAC ;; \
		*) echo "ERROR: unsupported target platform '$(TARGET_PLATFORM):$(TARGET_VARIANT)'" >&2; exit 1 ;; \
	esac; \
	if [ "$(AIO)" = "y" ]; then \
		config_y TCONFIG_AIO; \
	fi; \
	if [ "$(TARGET_PLATFORM)" = "MIPS" ]; then \
		if [ "$(AIO)" = "y" ]; then \
			config_n TCONFIG_OPTIMIZE_SIZE; \
		else \
			config_y TCONFIG_OPTIMIZE_SIZE; \
		fi; \
	elif [ "$(DO_OPTIMIZE_SIZE)" = "y" ]; then \
		config_y TCONFIG_OPTIMIZE_SIZE; \
	fi; \
	config_y TCONFIG_BONDING; \
	if [ "$(SLIM)" = "y" ]; then \
		config_n TCONFIG_EMF; \
	else \
		config_y TCONFIG_EMF; \
	fi; \
	if [ "$(JFFSv1)" = "y" ]; then \
		config_y TCONFIG_JFFSV1; \
	else \
		config_n TCONFIG_JFFSV1; \
	fi; \
	if [ "$(IPSEC)" = "y" ]; then \
		config_y TCONFIG_IPSEC; \
	else \
		config_n TCONFIG_IPSEC; \
	fi; \
	if [ "$(RAID)" = "y" ]; then \
		config_y TCONFIG_RAID; \
	else \
		config_n TCONFIG_RAID; \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		config_y TCONFIG_USB; \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			config_y TCONFIG_USB_EXTRAS; \
		fi; \
		if [ "$(NTFS)" = "y" ]; then \
			config_y TCONFIG_NTFS; \
		fi; \
		if [ "$(UFSD)" = "y" ]; then \
			config_y TCONFIG_UFSD; \
		elif [ "$(UFSD)" = "ASUS" ]; then \
			config_y TCONFIG_UFSDA; \
		fi; \
		if [ "$(TUXERA)" = "y" ]; then \
			config_y TCONFIG_TUXERA; \
		fi; \
		if [ "$(TUXERA_HFS)" = "y" ]; then \
			config_y TCONFIG_TUX_HFS; \
		fi; \
		if [ "$(ZFS)" = "y" ]; then \
			config_y TCONFIG_ZFS; \
		fi; \
		if [ "$(HFS)" = "y" ]; then \
			config_y TCONFIG_HFS; \
		fi; \
		if [ "$(MEDIASRV)" = "y" ]; then \
			config_y TCONFIG_MEDIA_SERVER; \
		fi; \
	else \
		config_n TCONFIG_USB; \
	fi; \
	if [ "$(NO_SAMBA)" = "y" ]; then \
		config_n TCONFIG_SAMBASRV; \
	fi; \
	if [ "$(NO_BONDING)" = "y" ]; then \
		config_n TCONFIG_BONDING; \
	fi; \
	if [ "$(NO_ZEBRA)" = "y" ]; then \
		config_n TCONFIG_ZEBRA; \
	fi; \
	if [ "$(NO_JFFS)" = "y" ]; then \
		config_n TCONFIG_JFFS2; \
	fi; \
	if [ "$(NO_CIFS)" = "y" ]; then \
		config_n TCONFIG_CIFS; \
	fi; \
	if [ "$(NO_HTTPS)" = "y" ]; then \
		config_n TCONFIG_HTTPS; \
	fi; \
	if [ "$(NO_ADBLOCK)" = "y" ]; then \
		config_n TCONFIG_ADBLOCK; \
	fi; \
	if [ "$(NO_DMZ)" = "y" ]; then \
		config_n TCONFIG_DMZ; \
	elif [ "$(NO_DMZMAC)" = "y" ]; then \
		config_n TCONFIG_DMZMAC; \
	fi; \
	if [ "$(NO_XXTP)" = "y" ]; then \
		config_n TCONFIG_L2TP; \
		config_n TCONFIG_PPTP; \
	fi; \
	if [ "$(NO_L7)" = "y" ]; then \
		config_n TCONFIG_L7; \
	elif [ "$(NO_L7PAT)" = "y" ]; then \
		config_n TCONFIG_L7PAT; \
	fi; \
	if [ "$(NO_LIBOPT)" = "y" ]; then \
		config_n TCONFIG_OPTIMIZE_SHARED_LIBS; \
	fi; \
	if [ "$(EBTABLES)" = "y" ]; then \
		config_y TCONFIG_EBTABLES; \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		config_y TCONFIG_IPV6; \
	fi; \
	if [ "$(NOCAT)" = "y" ]; then \
		config_y TCONFIG_NOCAT; \
	fi; \
	if [ "$(NGINX)" = "y" ]; then \
		config_y TCONFIG_NGINX; \
	fi; \
	if [ "$(FTPSSL)" = "y" ] && [ "$(NO_FTP)" = "y" ]; then \
		echo "ERROR: FTPSSL requires FTP; remove NO_FTP or FTPSSL" >&2; \
		exit 1; \
	fi; \
	if [ "$(FTPSSL)" = "y" ] && { [ "$(WOLFSSL)" = "y" ] || [ "$(WOLFSSLMIN)" = "y" ]; }; then \
		echo "ERROR: FTPSSL is not supported with WOLFSSL/WOLFSSLMIN" >&2; \
		exit 1; \
	fi; \
	if [ "$(FTPSSL)" = "y" ]; then \
		config_y TCONFIG_FTP_SSL; \
	fi; \
	if [ "$(OPENVPN)" = "y" ]; then \
		config_y TCONFIG_OPENVPN; \
	fi; \
	if [ "$(NO_FTP)" = "y" ]; then \
		config_n TCONFIG_FTP; \
		config_n TCONFIG_FTP_SSL; \
	fi; \
	if [ "$(WIREGUARD)" = "y" ]; then \
		config_y TCONFIG_WIREGUARD; \
	fi; \
	if [ "$(PPTPD)" = "y" ]; then \
		config_y TCONFIG_PPTPD; \
	fi; \
	if [ "$(BTCLIENT)" = "y" ]; then \
		config_y TCONFIG_BT; \
		config_y TCONFIG_BBT; \
	fi; \
	if [ "$(BTGUI)" = "y" ]; then \
		config_y TCONFIG_BT; \
	fi; \
	if [ "$(TR_EXTRAS)" = "y" ]; then \
		config_y TCONFIG_TR_EXTRAS; \
	fi; \
	if [ "$(NFS)" = "y" ]; then \
		config_y TCONFIG_NFS; \
	fi; \
	if [ "$(SNMP)" = "y" ]; then \
		config_y TCONFIG_SNMP; \
	fi; \
	if [ "$(SDHC)" = "y" ]; then \
		config_y TCONFIG_SDHC; \
	fi; \
	if [ "$(DNSSEC)" = "y" ]; then \
		config_y TCONFIG_DNSSEC; \
	fi; \
	if [ "$(UPS)" = "y" ]; then \
		config_y TCONFIG_UPS; \
	fi; \
	if [ "$(NANO)" = "y" ]; then \
		config_y TCONFIG_NANO; \
	fi; \
	if [ "$(DNSCRYPT)" = "y" ]; then \
		config_y TCONFIG_DNSCRYPT; \
	fi; \
	if [ "$(STUBBY)" = "y" ]; then \
		config_y TCONFIG_STUBBY; \
	fi; \
	if [ "$(NVRAM_64K)" = "y" ]; then \
		config_y TCONFIG_NVRAM_64K; \
	fi; \
	if [ "$(NVRAM_128K)" = "y" ]; then \
		config_y TCONFIG_NVRAM_128K; \
	fi; \
	if [ "$(DRAM_512M)" = "y" ]; then \
		config_y TCONFIG_DRAM_512M; \
	fi; \
	if [ "$(TOR)" = "y" ]; then \
		config_y TCONFIG_TOR; \
	fi; \
	if [ "$(MICROSD)" = "y" ]; then \
		config_y TCONFIG_MICROSD; \
	fi; \
	if [ "$(USBAP)" = "y" ]; then \
		config_y TCONFIG_USBAP; \
		if [ "$(NO_USBAPP)" = "y" ]; then \
			config_y TCONFIG_REMOVE_USBAPP; \
		fi; \
	fi; \
	if [ "$(CTF)" = "y" ]; then \
		config_y TCONFIG_CTF; \
	fi; \
	if [ "$(NAND)" = "y" ]; then \
		config_y TCONFIG_NAND; \
	fi; \
	if [ "$(AC3200)" = "y" ]; then \
		config_y TCONFIG_AC3200; \
	fi; \
	if [ "$(AC5300)" = "y" ]; then \
		config_y TCONFIG_AC5300; \
	fi; \
	if [ "$(BSD)" = "y" ]; then \
		config_y TCONFIG_BCMBSD; \
	fi; \
	if [ "$(ROAM)" = "y" ]; then \
		config_y TCONFIG_ROAM; \
	fi; \
	if [ "$(BCMNAT)" = "y" ]; then \
		config_y TCONFIG_BCMNAT; \
	fi; \
	if [ "$(BCMSMP)" = "y" ]; then \
		config_y TCONFIG_BCMSMP; \
	fi; \
	if [ "$(PROXYSTA)" = "y" ]; then \
		config_y TCONFIG_PROXYSTA; \
	fi; \
	if [ "$(DPSTA)" = "y" ]; then \
		config_y TCONFIG_DPSTA; \
	fi; \
	if [ "$(GRO)" = "y" ]; then \
		config_y TCONFIG_GROCTRL; \
	fi; \
	if [ "$(BCMFA)" = "y" ]; then \
		config_y TCONFIG_BCMFA; \
	fi; \
	if [ "$(TINC)" = "y" ]; then \
		config_y TCONFIG_TINC; \
	fi; \
	ssl_providers=0; \
	[ "$(OPENSSL11)" = "y" ] && ssl_providers=$$((ssl_providers + 1)); \
	[ "$(OPENSSL30)" = "y" ] && ssl_providers=$$((ssl_providers + 1)); \
	if [ "$(WOLFSSL)" = "y" ] || [ "$(WOLFSSLMIN)" = "y" ]; then ssl_providers=$$((ssl_providers + 1)); fi; \
	if [ "$$ssl_providers" -gt 1 ]; then \
		echo "ERROR: select only one TLS/SSL provider (OPENSSL11, OPENSSL30 or WOLFSSL/WOLFSSLMIN)" >&2; \
		exit 1; \
	fi; \
	if [ "$(OPENSSL11)" = "y" ]; then \
		config_y TCONFIG_OPENSSL11; \
	fi; \
	if [ "$(OPENSSL30)" = "y" ]; then \
		config_y TCONFIG_OPENSSL30; \
	fi; \
	if [ "$(WOLFSSL)" = "y" ]; then \
		config_y TCONFIG_WOLFSSL; \
	fi; \
	if [ "$(WOLFSSLMIN)" = "y" ]; then \
		config_y TCONFIG_WOLFSSL; \
		config_y TCONFIG_WOLFSSLMIN; \
	fi; \
	if [ "$(IPERF)" = "y" ]; then \
		config_y TCONFIG_IPERF; \
	fi; \
	if [ "$(KEYGEN)" = "y" ] && [ "$(NO_KEYGEN)" = "" ]; then \
		config_y TCONFIG_KEYGEN; \
	fi; \
	if [ "$(NVRAM_SIZE)" = "32" ]; then \
		config_y TCONFIG_NVRAM_32K; \
	fi; \
	if [ "$(BRCM_NAND_JFFS2)" = "y" ]; then \
		config_y TCONFIG_BRCM_NAND_JFFS2; \
	fi; \
	if [ "$(MDNS)" = "y" ]; then \
		config_y TCONFIG_MDNS; \
	fi; \
	if [ "$(PROXY)" = "y" ]; then \
		config_y TCONFIG_PROXY; \
	fi; \
	if [ "$(QRCODE)" = "y" ]; then \
		config_y TCONFIG_QRCODE; \
	fi; \
	if [ "$(FANCTRL)" = "y" ]; then \
		config_y TCONFIG_FANCTRL; \
	fi; \
	if [ "$(OPTIMIZE_SIZE_MORE)" = "y" ]; then \
		config_y TCONFIG_OPTIMIZE_SIZE_MORE; \
	fi; \
	if [ "$(CONNTRACK_TOOL)" = "y" ]; then \
		config_y TCONFIG_CONNTRACK_TOOL; \
	fi; \
	if [ "$(IRQBALANCE)" = "y" ]; then \
		config_y TCONFIG_IRQBALANCE; \
	fi; \
	if [ "$(NO_TRXKEY)" = "y" ]; then \
		config_n TCONFIG_TRXKEY; \
	fi; \
	if [ "$(EXTSW)" = "y" ]; then \
		config_y TCONFIG_EXTSW; \
	fi; \
	if [ "$(HAVEGED)" = "y" ]; then \
		config_y TCONFIG_HAVEGED; \
	fi; \
	)
endef

define BusyboxOptions
	@( \
	if [ "$(NO_CIFS)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_MOUNT_CIFS/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_CIFS is not set" >>$(1); \
	fi; \
	if [ "$(BBEXTRAS)" = "y" ]; then \
		sed -i "/CONFIG_SENDMAIL/d" $(1); \
		echo "CONFIG_SENDMAIL=y" >>$(1); \
		sed -i "/CONFIG_WHOIS/d" $(1); \
		echo "CONFIG_WHOIS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_SORT_BIG/d" $(1); \
		echo "CONFIG_FEATURE_SORT_BIG=y" >>$(1); \
		sed -i "/CONFIG_CLEAR/d" $(1); \
		echo "CONFIG_CLEAR=y" >>$(1); \
		sed -i "/CONFIG_NICE/d" $(1); \
		echo "CONFIG_NICE=y" >>$(1); \
		sed -i "/CONFIG_SETCONSOLE/d" $(1); \
		echo "CONFIG_SETCONSOLE=y" >>$(1); \
		sed -i "/CONFIG_MKFIFO/d" $(1); \
		echo "CONFIG_MKFIFO=y" >>$(1); \
		sed -i "/CONFIG_SEQ/d" $(1); \
		echo "CONFIG_SEQ=y" >>$(1); \
		sed -i "/CONFIG_STTY/d" $(1); \
		echo "CONFIG_STTY=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/CONFIG_FDISK/d" $(1); \
			echo "CONFIG_FDISK=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_FDISK_WRITABLE/d" $(1); \
			echo "CONFIG_FEATURE_FDISK_WRITABLE=y" >>$(1); \
			sed -i "/CONFIG_FDISK_SUPPORT_LARGE_DISKS/d" $(1); \
			echo "CONFIG_FDISK_SUPPORT_LARGE_DISKS=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_GPT_LABEL/d" $(1); \
			echo "CONFIG_FEATURE_GPT_LABEL=y" >>$(1); \
			sed -i "/CONFIG_MKFS_VFAT/d" $(1); \
			echo "CONFIG_MKFS_VFAT=y" >>$(1); \
			sed -i "/CONFIG_MKSWAP/d" $(1); \
			echo "CONFIG_MKSWAP=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_MKSWAP_UUID/d" $(1); \
			echo "CONFIG_FEATURE_MKSWAP_UUID=y" >>$(1); \
			sed -i "/CONFIG_FSYNC/d" $(1); \
			echo "CONFIG_FSYNC=y" >>$(1); \
			sed -i "/CONFIG_LSUSB/d" $(1); \
			echo "CONFIG_LSUSB=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_VERBOSE_USAGE/d" $(1); \
			echo "CONFIG_FEATURE_VERBOSE_USAGE=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_VOLUMEID_EXFAT/d" $(1); \
			echo "CONFIG_FEATURE_VOLUMEID_EXFAT=y" >>$(1); \
		fi; \
	else \
		sed -i "/CONFIG_FEATURE_MOUNT_LOOP/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LOOP is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_LABEL/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LABEL is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_FSTAB/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_FSTAB is not set" >>$(1); \
		sed -i "/CONFIG_VOLUMEID/d" $(1); \
		echo "# CONFIG_VOLUMEID is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_SWAPON/d" $(1); \
		echo "# CONFIG_SWAPON is not set" >>$(1); \
		sed -i "/CONFIG_SWAPOFF/d" $(1); \
		echo "# CONFIG_SWAPOFF is not set" >>$(1); \
		sed -i "/CONFIG_CHROOT/d" $(1); \
		echo "# CONFIG_CHROOT is not set" >>$(1); \
		sed -i "/CONFIG_PIVOT_ROOT/d" $(1); \
		echo "# CONFIG_PIVOT_ROOT is not set" >>$(1); \
		sed -i "/CONFIG_TRUE/d" $(1); \
		echo "# CONFIG_TRUE is not set" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_IPV6/d" $(1); \
		echo "CONFIG_FEATURE_IPV6=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_PREFER_IPV4_ADDRESS/d" $(1); \
		echo "CONFIG_FEATURE_PREFER_IPV4_ADDRESS=y" >>$(1); \
		sed -i "/CONFIG_PING6/d" $(1); \
		echo "CONFIG_PING6=y" >>$(1); \
		sed -i "/CONFIG_TRACEROUTE6/d" $(1); \
		echo "CONFIG_TRACEROUTE6=y" >>$(1); \
	fi; \
	if [ "$(SLIM)" = "y" ]; then \
		sed -i "/CONFIG_AWK/d" $(1); \
		echo "# CONFIG_AWK is not set" >>$(1); \
		sed -i "/CONFIG_BASENAME/d" $(1); \
		echo "# CONFIG_BASENAME is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_TELNET=y/d" $(1); \
		echo "# CONFIG_TELNET is not set" >>$(1); \
		sed -i "/CONFIG_ARPING/d" $(1); \
		echo "# CONFIG_ARPING is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR/d" $(1); \
		echo "# CONFIG_FEATURE_LS_COLOR is not set" >>$(1); \
		sed -i "/CONFIG_CHOWN/d" $(1); \
		echo "# CONFIG_CHOWN is not set" >>$(1); \
	else \
		sed -i "/CONFIG_FEATURE_LS_COLOR/d" $(1); \
		echo "CONFIG_FEATURE_LS_COLOR=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR_IS_DEFAULT/d" $(1); \
		echo "CONFIG_FEATURE_LS_COLOR_IS_DEFAULT=y" >>$(1); \
	fi; \
	if [ "$(BCMSMP)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_TOP_SMP_CPU/d" $(1); \
		echo "CONFIG_FEATURE_TOP_SMP_CPU=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOP_DECIMALS/d" $(1); \
		echo "CONFIG_FEATURE_TOP_DECIMALS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOP_SMP_PROCESS/d" $(1); \
		echo "CONFIG_FEATURE_TOP_SMP_PROCESS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOPMEM/d" $(1); \
		echo "CONFIG_FEATURE_TOPMEM=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_SHOW_THREADS/d" $(1); \
		echo "CONFIG_FEATURE_SHOW_THREADS=y" >>$(1); \
	fi; \
	if [ "$(FANCTRL)" = "y" ]; then \
		sed -i "/CONFIG_RENICE/d" $(1); \
		echo "CONFIG_RENICE=y" >>$(1); \
	fi; \
	if [ "$(WIREGUARD)" = "y" ]; then \
		sed -i "s/# CONFIG_MKTEMP is not set/CONFIG_MKTEMP=y/g" $(1); \
		sed -i "s/# CONFIG_READLINK is not set/CONFIG_READLINK=y/g" $(1); \
		sed -i "s/# CONFIG_FEATURE_READLINK_FOLLOW is not set/CONFIG_FEATURE_READLINK_FOLLOW=y/g" $(1); \
		sed -i "s/# CONFIG_FEATURE_TR_CLASSES is not set/CONFIG_FEATURE_TR_CLASSES=y/g" $(1); \
		sed -i "s/# CONFIG_STAT is not set/CONFIG_STAT=y/g" $(1); \
		sed -i "s/# CONFIG_FEATURE_STAT_FORMAT is not set/CONFIG_FEATURE_STAT_FORMAT=y/g" $(1); \
	fi; \
	)
endef

define extrawlConfig
	@( \
	if [ "$(CONFIG_BCM714)" = "y" ]; then \
		cp -f $(SRCBASE)/router/dpsta/arm_714/dpsta.h $(SRCBASE)/router/dpsta/dpsta.h ; \
		cp -f $(SRCBASE)/router/dpsta/arm_714/dpsta_linux.h $(SRCBASE)/router/dpsta/dpsta_linux.h ; \
		cp -f $(SRCBASE)/router/dpsta/arm_714/dpsta.o $(SRCBASE)/router/dpsta/linux/dpsta.o ; \
	else \
		cp -f $(SRCBASE)/router/dpsta/arm/dpsta.h $(SRCBASE)/router/dpsta/dpsta.h ; \
		cp -f $(SRCBASE)/router/dpsta/arm/dpsta_linux.h $(SRCBASE)/router/dpsta/dpsta_linux.h ; \
		cp -f $(SRCBASE)/router/dpsta/arm/dpsta.o $(SRCBASE)/router/dpsta/linux/dpsta.o ; \
		if [ "$(CONFIG_BCM7)" = "y" ]; then \
			if [ "$(BCM7WL2020)" = "y" ]; then \
				echo "Using SDK7 WL Driver - Year 2020"; \
				cp -f $(SRCBASE)/router/dhd/linux/2020/dhd.o $(SRCBASE)/router/dhd/linux/dhd.o ; \
			elif [ "$(BCM7WL2021)" = "y" ]; then \
				echo "Using SDK7 WL Driver - Year 2021"; \
				cp -f $(SRCBASE)/router/dhd/linux/2021/dhd.o $(SRCBASE)/router/dhd/linux/dhd.o ; \
			else \
				echo "Using SDK7 WL Driver - Year 2023 (default)"; \
				cp -f $(SRCBASE)/router/dhd/linux/2023/dhd.o $(SRCBASE)/router/dhd/linux/dhd.o ; \
			fi; \
		fi; \
	fi; \
	)
endef

define KernelConfig
	@( \
	sed -i "/CONFIG_NVRAM_SIZE/d" $(1); \
	echo "CONFIG_NVRAM_SIZE="$(NVRAM_SIZE) >>$(1); \
	sed -i "/CONFIG_CC_OPTIMIZE_FOR_SIZE/d" $(1); \
	if [ "$(KERN_SIZE_OPT)" = "y" ]; then \
		echo "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" >>$(1); \
	else \
		echo "# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set" >>$(1); \
	fi; \
	if [ "$(NAND)" = "y" ]; then \
		sed -i "/CONFIG_MTD_NFLASH/d" $(1); \
		echo "CONFIG_MTD_NFLASH=y" >>$(1); \
		sed -i "/CONFIG_MTD_NAND/d" $(1); \
		echo "CONFIG_MTD_NAND=y" >>$(1); \
		echo "CONFIG_MTD_NAND_IDS=y" >>$(1); \
		echo "# CONFIG_MTD_NAND_DENALI is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_RICOH is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_VERIFY_WRITE is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_ECC_SMC is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_MUSEUM_IDS is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_DISKONCHIP is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_CAFE is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_NANDSIM is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_PLATFORM is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_ONENAND is not set" >>$(1); \
		sed -i "/CONFIG_MTD_BRCMNAND/d" $(1); \
		echo "CONFIG_MTD_BRCMNAND=y" >>$(1); \
	fi; \
	if [ "$(SPI)" = "y" ]; then \
		echo "# CONFIG_MTD_DATAFLASH is not set" >>$(1); \
		echo "CONFIG_MTD_M25P80=y" >>$(1); \
		echo "CONFIG_M25PXX_USE_FAST_READ=y" >>$(1); \
		echo "# CONFIG_MTD_SST25L is not set" >>$(1); \
		echo "# CONFIG_AD525X_DPOT is not set" >>$(1); \
		echo "# CONFIG_TI_DAC7512 is not set" >>$(1); \
		echo "# CONFIG_EEPROM_AT25 is not set" >>$(1); \
		echo "# CONFIG_SERIAL_MAX3100 is not set" >>$(1); \
		echo "# CONFIG_SERIAL_MAX3107 is not set" >>$(1); \
		sed -i "s/# CONFIG_SPI is not set/CONFIG_SPI=y/g" $(1); \
		echo "CONFIG_SPI_MASTER=y" >>$(1); \
		echo "# SPI Master Controller Drivers" >>$(1); \
		echo "# CONFIG_SPI_BITBANG is not set" >>$(1); \
		echo "# CONFIG_SPI_XILINX is not set" >>$(1); \
		echo "# CONFIG_SPI_DESIGNWARE is not set" >>$(1); \
		echo "# CONFIG_SPI_SPIDEV is not set" >>$(1); \
		echo "# CONFIG_SPI_TLE62X0 is not set" >>$(1); \
		echo "# CONFIG_SND_SPI is not set" >>$(1); \
		echo "# CONFIG_SND_SOC is not set" >>$(1); \
		echo "# CONFIG_LEDS_DAC124S085 is not set" >>$(1); \
	fi; \
	if [ "$(UPS)" = "y" ]; then \
		sed -i "s/# CONFIG_HIDRAW is not set/CONFIG_HIDRAW=y/g" $(1); \
		sed -i "/CONFIG_USB_STORAGE_ONETOUCH/d" $(1); \
		echo "# CONFIG_USB_STORAGE_ONETOUCH is not set" >>$(1); \
	else \
		sed -i "/CONFIG_USB_STORAGE_ONETOUCH/d" $(1); \
		echo "# CONFIG_USB_STORAGE_ONETOUCH is not set" >>$(1); \
	fi; \
	if [ "$(USB)" = "" ]; then \
		sed -i "/CONFIG_EFI_PARTITION/d" $(1); \
		echo "# CONFIG_EFI_PARTITION is not set" >>$(1); \
	else \
		if [ "$(HFS)" = "y" ]; then \
			sed -i "s/# CONFIG_HFS_FS is not set/CONFIG_HFS_FS=m/g" $(1); \
			sed -i "s/# CONFIG_HFSPLUS_FS is not set/CONFIG_HFSPLUS_FS=m/g" $(1); \
		fi; \
	fi; \
	if [ "$(AC3200)" = "y" ] || [ "$(AC5300)" = "y" ]; then \
		sed -i "/CONFIG_RTAC3200/d" $(1); \
		echo "CONFIG_RTAC3200=y" >>$(1); \
	fi; \
	if [ "$(CONFIG_BCM7)" = "y" ]; then \
		sed -i "/CONFIG_BCM_GMAC3/d" $(1); \
		echo "CONFIG_BCM_GMAC3=y" >>$(1); \
	else \
		sed -i "/CONFIG_BCM_GMAC3/d" $(1); \
		echo "# CONFIG_BCM_GMAC3 is not set" >>$(1); \
	fi; \
	if [ "$(BCM_RECVFILE)" = "y" ]; then \
		sed -i "/CONFIG_BCM_RECVFILE/d" $(1); \
		echo "CONFIG_BCM_RECVFILE=y" >>$(1); \
	fi; \
	if [ "$(DEBUGFS)" = "y" ]; then \
		sed -i "/CONFIG_DEBUG_FS/d" $(1); \
		echo "CONFIG_DEBUG_FS=y" >>$(1); \
		sed -i "/CONFIG_USB_MON/d" $(1); \
		echo "CONFIG_USB_MON=m" >>$(1); \
		sed -i "/CONFIG_GCOV_KERNEL/d" $(1); \
		echo "# CONFIG_GCOV_KERNEL is not set" >>$(1); \
		sed -i "/CONFIG_L2TP_DEBUGFS/d" $(1); \
		echo "# CONFIG_L2TP_DEBUGFS is not set" >>$(1); \
		sed -i "/CONFIG_JBD_DEBUG/d" $(1); \
		echo "# CONFIG_JBD_DEBUG is not set" >>$(1); \
		sed -i "/CONFIG_JBD2_DEBUG/d" $(1); \
		echo "# CONFIG_JBD2_DEBUG is not set" >>$(1); \
		sed -i "/CONFIG_LKDTM/d" $(1); \
		echo "# CONFIG_LKDTM is not set" >>$(1); \
		sed -i "/CONFIG_DYNAMIC_DEBUG/d" $(1); \
		echo "# CONFIG_DYNAMIC_DEBUG is not set" >>$(1); \
	fi; \
	if [ "$(NOWL)" = "y" ]; then \
		sed -i "/CONFIG_WL=m/d" $(1); \
		echo "# CONFIG_WL is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_APSTA/d" $(1); \
		echo "# CONFIG_WL_USE_APSTA is not set" >>$(1); \
		sed -i "/CONFIG_WL_ALL_PASSIVE_RUNTIME/d" $(1); \
		sed -i "/CONFIG_WAPI/d" $(1); \
		echo "# CONFIG_WAPI is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP/d" $(1); \
		echo "# CONFIG_WL_USE_AP is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP_SDSTD/d" $(1); \
		echo "# CONFIG_WL_USE_AP_SDSTD is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_STA/d" $(1); \
		echo "# CONFIG_WL_USE_STA is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_AP_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_STA_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_STA_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_APSTA_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_APSTA_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_ALL_PASSIVE_ON/d" $(1); \
		echo "# CONFIG_WL_ALL_PASSIVE_ON is not set" >>$(1); \
		sed -i "/CONFIG_DPSTA/d" $(1); \
		echo "# CONFIG_DPSTA is not set" >>$(1); \
		sed -i "/CONFIG_PLC/d" $(1); \
		echo "# CONFIG_PLC is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_BONDING/d" $(1); \
	if [ "$(NO_BONDING)" = "y" ]; then \
		echo "# CONFIG_BONDING is not set" >>$(1); \
	else \
		echo "CONFIG_BONDING=m" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "s/# CONFIG_IPV6 is not set/CONFIG_IPV6=y/g" $(1); \
		sed -i "/CONFIG_IP6_NF_IPTABLES/d" $(1); \
		echo "CONFIG_IP6_NF_IPTABLES=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MATCH_RT/d" $(1); \
		echo "CONFIG_IP6_NF_MATCH_RT=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_FILTER/d" $(1); \
		echo "CONFIG_IP6_NF_FILTER=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_LOG/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_LOG=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_REJECT/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_REJECT=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MANGLE/d" $(1); \
		echo "CONFIG_IP6_NF_MANGLE=m" >>$(1); \
		sed -i "/CONFIG_NF_CONNTRACK_IPV6/d" $(1); \
		echo "CONFIG_NF_CONNTRACK_IPV6=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_HL/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_HL=m" >>$(1); \
		sed -i "/CONFIG_IPV6_ROUTER_PREF/d" $(1); \
		echo "CONFIG_IPV6_ROUTER_PREF=y" >>$(1); \
		sed -i "/CONFIG_IPV6_SIT/d" $(1); \
		echo "CONFIG_IPV6_SIT=m" >>$(1); \
		sed -i "/CONFIG_IPV6_SIT_6RD/d" $(1); \
		echo "CONFIG_IPV6_SIT_6RD=y" >>$(1); \
		sed -i "/CONFIG_IPV6_MULTIPLE_TABLES/d" $(1); \
		echo "CONFIG_IPV6_MULTIPLE_TABLES=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_ROUTE/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_ROUTE=m" >>$(1); \
		sed -i "/CONFIG_IPV6_MROUTE/d" $(1); \
		echo "CONFIG_IPV6_MROUTE=y" >>$(1); \
		echo "# CONFIG_IPV6_MROUTE_MULTIPLE_TABLES is not set" >>$(1); \
		if [ "$(CONFIG_BCM7)" = "" ]; then \
			sed -i "/CONFIG_IP6_NF_RAW/d" $(1); \
			echo "CONFIG_IP6_NF_RAW=m" >>$(1); \
			sed -i "/CONFIG_IPV6_OPTIMISTIC_DAD/d" $(1); \
			echo "CONFIG_IPV6_OPTIMISTIC_DAD=y" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_TUNNEL/d" $(1); \
			echo "CONFIG_INET6_XFRM_TUNNEL=m" >>$(1); \
			sed -i "/CONFIG_INET6_AH/d" $(1); \
			echo "CONFIG_INET6_AH=m" >>$(1); \
			sed -i "/CONFIG_INET6_ESP/d" $(1); \
			echo "CONFIG_INET6_ESP=m" >>$(1); \
			sed -i "/CONFIG_INET6_IPCOMP/d" $(1); \
			echo "CONFIG_INET6_IPCOMP=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TRANSPORT/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TRANSPORT=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TUNNEL/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TUNNEL=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_BEET/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_BEET=m" >>$(1); \
		fi; \
	fi; \
	if [ "$(IPSEC)" = "y" ] && [ "$(CONFIG_BCM7)" = "y" ]; then \
		sed -i "/CONFIG_XFRM_USER/d" $(1); \
		echo "CONFIG_XFRM_USER=m" >>$(1); \
		echo "# CONFIG_XFRM_SUB_POLICY is not set" >>$(1); \
		echo "# CONFIG_XFRM_MIGRATE is not set" >>$(1); \
		echo "# CONFIG_XFRM_STATISTICS is not set" >>$(1); \
		sed -i "/CONFIG_NET_KEY/d" $(1); \
		echo "CONFIG_NET_KEY=m" >>$(1); \
		echo "# CONFIG_NET_KEY_MIGRATE is not set" >>$(1); \
		sed -i "/CONFIG_INET_AH/d" $(1); \
		echo "CONFIG_INET_AH=m" >>$(1); \
		sed -i "/CONFIG_INET_ESP/d" $(1); \
		echo "CONFIG_INET_ESP=m" >>$(1); \
		sed -i "/CONFIG_INET_IPCOMP/d" $(1); \
		echo "CONFIG_INET_IPCOMP=m" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_MODE_TRANSPORT/d" $(1); \
		echo "CONFIG_INET_XFRM_MODE_TRANSPORT=m" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_MODE_TUNNEL/d" $(1); \
		echo "CONFIG_INET_XFRM_MODE_TUNNEL=m" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_MODE_BEET/d" $(1); \
		echo "CONFIG_INET_XFRM_MODE_BEET=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_POLICY/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_POLICY=m" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_INET6_AH/d" $(1); \
			echo "CONFIG_INET6_AH=m" >>$(1); \
			sed -i "/CONFIG_INET6_ESP/d" $(1); \
			echo "CONFIG_INET6_ESP=m" >>$(1); \
			sed -i "/CONFIG_INET6_IPCOMP/d" $(1); \
			echo "CONFIG_INET6_IPCOMP=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TRANSPORT/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TRANSPORT=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TUNNEL/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TUNNEL=m" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_BEET/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_BEET=m" >>$(1); \
		fi; \
	fi; \
	sed -i "/CONFIG_BCM57XX/d" $(1); \
	if [ "$(BCM57)" = "y" ]; then \
		echo "CONFIG_BCM57XX=m" >>$(1); \
	else \
		echo "# CONFIG_BCM57XX is not set" >>$(1); \
	fi; \
	if [ "$(BCM_714)" = "y" ]; then \
		sed -i "/CONFIG_CR4_OFFLOAD/d" $(1); \
		echo "# CONFIG_CR4_OFFLOAD is not set" >>$(1); \
		sed -i "/CONFIG_PLAT_UART_CLOCKS/d" $(1); \
		echo "# CONFIG_PLAT_UART_CLOCKS is not set" >>$(1); \
	fi; \
	if [ "$(DPSTA)" = "y" ]; then \
		sed -i "/CONFIG_DPSTA/d" $(1); \
		echo "CONFIG_DPSTA=m" >>$(1); \
	fi; \
	sed -i "/CONFIG_LINUX_MTD/d" $(1); \
	if [ "$(LINUX_MTD)" = "" ]; then \
		echo "CONFIG_LINUX_MTD=32" >>$(1); \
	else \
		echo "CONFIG_LINUX_MTD=$(LINUX_MTD)" >>$(1); \
	fi; \
	if [ "$(ARMCPUSMP)" = "up" ]; then \
		sed -i "/CONFIG_GENERIC_CLOCKEVENTS_BROADCAST/d" $(1); \
		echo "CONFIG_HAVE_LATENCYTOP_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_GENERIC_LOCKBREAK/d" $(1); \
		echo "CONFIG_BROKEN_ON_SMP=y" >>$(1); \
		sed -i "s/CONFIG_TREE_RCU=y/# CONFIG_TREE_RCU is not set/g" $(1); \
		echo "CONFIG_TREE_PREEMPT_RCU=y" >>$(1); \
		echo "# CONFIG_TINY_RCU is not set" >>$(1); \
		sed -i "/CONFIG_USE_GENERIC_SMP_HELPERS/d" $(1); \
		sed -i "/CONFIG_STOP_MACHINE/d" $(1); \
		sed -i "/CONFIG_MUTEX_SPIN_ON_OWNER/d" $(1); \
		echo "# CONFIG_MUTEX_SPIN_ON_OWNER is not set" >>$(1); \
		sed -i "/# CONFIG_ARM_ERRATA_742230 is not set/d" $(1); \
		sed -i "/# CONFIG_ARM_ERRATA_742231 is not set/d" $(1); \
		sed -i "/# CONFIG_ARM_ERRATA_720789 is not set/d" $(1); \
		sed -i "/CONFIG_SMP=y/d" $(1); \
		echo "# CONFIG_SMP is not set" >>$(1); \
		sed -i "/CONFIG_NR_CPUS=2/d" $(1); \
		sed -i "/# CONFIG_HOTPLUG_CPU is not set/d" $(1); \
		sed -i "/CONFIG_RPS=y/d" $(1); \
	fi; \
	if [ "$(USBAP)" = "y" ]; then \
		sed -i "s/# CONFIG_WL_USBAP is not set/CONFIG_WL_USBAP=y/g" $(1); \
		sed -i "s/CONFIG_WL_APSTA="wlconfig_lx_router_apsta"/CONFIG_WL_APSTA="wlconfig_lx_router_high"/g" $(1); \
	else \
		sed -i "s/CONFIG_WL_USBAP=y/# CONFIG_WL_USBAP is not set/g" $(1); \
		sed -i "s/CONFIG_WL_APSTA="wlconfig_lx_router_high"/CONFIG_WL_APSTA="wlconfig_lx_router_apsta"/g" $(1); \
	fi; \
	if [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/CONFIG_BRIDGE_NF_EBTABLES/d" $(1); \
		echo "CONFIG_BRIDGE_NF_EBTABLES=m" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_BRIDGE_EBT_IP6/d" $(1); \
			echo "CONFIG_BRIDGE_EBT_IP6=m" >>$(1); \
		fi; \
	fi; \
	sed -i "/CONFIG_NVRAM_64K/d" $(1); \
	if [ "$(NVRAM_64K)" = "y" ]; then \
		echo "CONFIG_NVRAM_64K=y" >>$(1); \
	else \
		echo "# CONFIG_NVRAM_64K is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_NVRAM_128K/d" $(1); \
	if [ "$(NVRAM_128K)" = "y" ]; then \
		echo "CONFIG_NVRAM_128K=y" >>$(1); \
	else \
		echo "# CONFIG_NVRAM_128K is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_DRAM_512M/d" $(1); \
	if [ "$(DRAM_512M)" = "y" ]; then \
		echo "CONFIG_DRAM_512M=y" >>$(1); \
	else \
		echo "# CONFIG_DRAM_512M is not set" >>$(1); \
	fi; \
	if [ "$(SATA)" = "y" ]; then \
		sed -i "/CONFIG_CMDLINE/d" $(1); \
		echo "CONFIG_CMDLINE=\"root=\/dev\/mtdblock2 console=ttyS0,115200 init=\/sbin\/preinit earlyprintk debug libata.force=3.0Gbps\"" >>$(1); \
		echo "# CONFIG_CMDLINE_FORCE is not set" >>$(1); \
		sed -i "/CONFIG_SCSI_MOD/d" $(1); \
		echo "CONFIG_SCSI_MOD=y" >>$(1); \
		sed -i "/CONFIG_SCSI=m/d" $(1); \
		echo "CONFIG_SCSI=y" >>$(1); \
		sed -i "/CONFIG_SCSI_PROC_FS/d" $(1); \
		echo "CONFIG_SCSI_PROC_FS=y" >>$(1); \
		sed -i "/CONFIG_SCSI_MULTI_LUN/d" $(1); \
		echo "CONFIG_SCSI_MULTI_LUN=y" >>$(1); \
		sed -i "/CONFIG_SCSI_CONSTANTS/d" $(1); \
		echo "CONFIG_SCSI_CONSTANTS=y" >>$(1); \
		sed -i "/CONFIG_SCSI_LOWLEVEL/d" $(1); \
		echo "CONFIG_SCSI_LOWLEVEL=y" >>$(1); \
		sed -i "/CONFIG_BLK_DEV_SD/d" $(1); \
		echo "CONFIG_BLK_DEV_SD=y" >>$(1); \
		sed -i "/CONFIG_ATA is not set/d" $(1); \
		echo "CONFIG_ATA=y" >>$(1); \
		sed -i "/CONFIG_SATA_PMP/d" $(1); \
		echo "CONFIG_SATA_PMP=y" >>$(1); \
		sed -i "/CONFIG_SATA_AHCI/d" $(1); \
		echo "CONFIG_SATA_AHCI=y" >>$(1); \
		sed -i "/CONFIG_SATA_AHCI_PLATFORM/d" $(1); \
		echo "CONFIG_SATA_AHCI_PLATFORM=y" >>$(1); \
		sed -i "/CONFIG_ATA_SFF/d" $(1); \
		echo "CONFIG_ATA_SFF=y" >>$(1); \
		sed -i "/CONFIG_ATA_BMDMA/d" $(1); \
		echo "CONFIG_ATA_BMDMA=y" >>$(1); \
		sed -i "/CONFIG_ATA_VERBOSE_ERROR/d" $(1); \
		echo "# CONFIG_ATA_VERBOSE_ERROR is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_EXT2_/d" $(1); \
	sed -i "/CONFIG_EXT3_/d" $(1); \
	sed -i "/CONFIG_EXT4_/d" $(1); \
	echo "# CONFIG_EXT2_FS is not set" >>$(1); \
	echo "# CONFIG_EXT3_FS is not set" >>$(1); \
	echo "CONFIG_EXT4_FS=m" >>$(1); \
	echo "CONFIG_EXT4_USE_FOR_EXT23=y" >>$(1); \
	echo "CONFIG_EXT4_FS_XATTR=y" >>$(1); \
	echo "# CONFIG_EXT4_FS_POSIX_ACL is not set" >>$(1); \
	echo "# CONFIG_EXT4_FS_SECURITY is not set" >>$(1); \
	echo "# CONFIG_EXT4_DEBUG is not set" >>$(1); \
	sed -i "/CONFIG_FAILSAFE_UPGRADE/d" $(1); \
	if [ "$(FAILSAFE_UPGRADE)" = "y" ]; then \
		echo "CONFIG_FAILSAFE_UPGRADE=y" >>$(1); \
	else \
		echo "# CONFIG_FAILSAFE_UPGRADE is not set" >> $(1); \
	fi; \
	if [ "$(CRASH_LOG)" = "y" ]; then \
		sed -i "/CONFIG_CRASHLOG/d" $(1); \
		echo "CONFIG_CRASHLOG=y" >>$(1); \
		sed -i "/BCMDBG/d" $(1); \
		echo "BCMDBG=y" >>$(1); \
		sed -i "/CONFIG_DEBUG_INFO/d" $(1); \
		echo "CONFIG_DEBUG_INFO=y" >>$(1); \
	else \
		sed -i "/CONFIG_CRASHLOG/d" $(1); \
		echo "# CONFIG_CRASHLOG is not set" >>$(1); \
		sed -i "/BCMDBG/d" $(1); \
		echo "# BCMDBG is not set" >>$(1); \
		sed -i "/CONFIG_DEBUG_INFO/d" $(1); \
		echo "# CONFIG_DEBUG_INFO is not set" >>$(1); \
	fi; \
	if [ "$(WIREGUARD)" = "y" ]; then \
		sed -i "s/# CONFIG_WIREGUARD is not set/CONFIG_WIREGUARD=m/g" $(1); \
	fi; \
	if [ "$(ZFS)" = "y" ]; then \
		sed -i "s/# CONFIG_ZFS is not set/CONFIG_ZFS=m/g" $(1); \
	fi; \
	)
	$(call extrawlConfig, $(1))
endef

bin:
ifeq ($(B),)
	@echo $@" is not a valid target!"
	@false
endif
	@cp router/config_base router/config_$(lowercase_B)
	@cp router/busybox/config_base router/busybox/config_$(lowercase_B)
	@cp $(LINUXDIR)/config_base $(LINUXDIR)/config_$(lowercase_B)

	$(call RouterOptions, router/config_$(lowercase_B))
	$(call KernelConfig, $(LINUXDIR)/config_$(lowercase_B))
	$(call BusyboxOptions, router/busybox/config_$(lowercase_B))

	@$(MAKE) setprofile N=$(TPROFILE) B=$(B) DESC="$(BUILD_DESC)" USB="$(USB)"
ifneq ($(CONFIG_ONLY),y)
	@$(MAKE) all
endif

## basic targets
e:
	@$(MAKE) bin MWAN_MAX=$(if $(MWAN_MAX),$(MWAN_MAX),4) B=E USB="USB" IPV6SUPP=y CTF=y GRO=y PROXY=y BBEXTRAS=y USBEXTRAS=y PPTPD=y DNSSEC=y NTFS=y HFS=y EBTABLES=y OPENSSL30=y OPENVPN=y FTPSSL=y KEYGEN=y WIREGUARD=y MEDIASRV=y ROAM=y \
	     QRCODE=y STUBBY=y HAVEGED=y TINC=y BUILD_DESC="$(VPN)"

o:
	@$(MAKE) bin MWAN_MAX=$(if $(MWAN_MAX),$(MWAN_MAX),4) B=E USB="USB" IPV6SUPP=y CTF=y GRO=y PROXY=y BBEXTRAS=y USBEXTRAS=y PPTPD=y DNSSEC=y NTFS=y HFS=y EBTABLES=y OPENSSL30=y OPENVPN=y FTPSSL=y KEYGEN=y WIREGUARD=y MEDIASRV=y ROAM=y \
	    QRCODE=y STUBBY=y HAVEGED=y DNSCRYPT=y TINC=y NFS=y IPERF=y MDNS=y BUILD_DESC="Custom"

y:
	@$(MAKE) bin MWAN_MAX=$(if $(MWAN_MAX),$(MWAN_MAX),4) B=E USB="USB" IPV6SUPP=y CTF=y GRO=y PROXY=y BBEXTRAS=y USBEXTRAS=y PPTPD=y DNSSEC=y NTFS=y HFS=y EBTABLES=y OPENSSL30=y OPENVPN=y FTPSSL=y KEYGEN=y WIREGUARD=y MEDIASRV=y ROAM=y \
	    QRCODE=y STUBBY=y HAVEGED=y DNSCRYPT=y TINC=y NFS=y IPERF=y MDNS=y BTCLIENT=y TR_EXTRAS=y NOCAT=y TOR=y NGINX=y NANO=y CRASH_LOG=y BUILD_DESC="AIO_Lite"

z:
	@$(MAKE) bin MWAN_MAX=$(if $(MWAN_MAX),$(MWAN_MAX),4) B=E USB="USB" IPV6SUPP=y CTF=y GRO=y PROXY=y BBEXTRAS=y USBEXTRAS=y PPTPD=y DNSSEC=y NTFS=y HFS=y EBTABLES=y OPENSSL30=y OPENVPN=y FTPSSL=y KEYGEN=y WIREGUARD=y MEDIASRV=y ROAM=y \
	    QRCODE=y STUBBY=y HAVEGED=y DNSCRYPT=y TINC=y NFS=y IPERF=y MDNS=y BTCLIENT=y TR_EXTRAS=y NOCAT=y TOR=y NGINX=y NANO=y CRASH_LOG=y SNMP=y UPS=y ZFS=y CONNTRACK_TOOL=y AIO=y BUILD_DESC="AIO"

include targets.mak

setprofile:
	echo '#ifndef TOMATO_PROFILE' > router/shared/tomato_profile.h
	echo '#define TOMATO_$(N) 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_G 1' >> router/shared/tomato_profile.h
	echo '#define PROFILE_N 2' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE PROFILE_$(N)' >> router/shared/tomato_profile.h
	echo '#define TOMATO_PROFILE_NAME "$(N)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_NAME "$(B)"' >> router/shared/tomato_profile.h
	echo '#define TOMATO_BUILD_DESC "$(DESC)$(EXTRA_64KDESC)$(NOSMP_DESC)$(SDK7WL_DESC)"' >> router/shared/tomato_profile.h
	echo '#ifndef CONFIG_NVRAM_SIZE' >> router/shared/tomato_profile.h
	echo '#define CONFIG_NVRAM_SIZE $(NVRAM_SIZE)' >> router/shared/tomato_profile.h
	echo '#endif' >> router/shared/tomato_profile.h
	echo '#endif' >> router/shared/tomato_profile.h

	echo 'TOMATO_$(N) = 1' > tomato_profile.mak
	echo 'PROFILE_G = 1' >> tomato_profile.mak
	echo 'PROFILE_N = 2' >> tomato_profile.mak
	echo 'TOMATO_PROFILE = $$(PROFILE_$(N))' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_NAME = "$(N)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD = "$(B)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD_NAME = "$(B)"' >> tomato_profile.mak
	echo 'TOMATO_BUILD_DESC = "$(DESC)$(EXTRA_64KDESC)$(NOSMP_DESC)$(SDK7WL_DESC)"' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_L = $(lowercase_N)' >> tomato_profile.mak
	echo 'TOMATO_PROFILE_U = $(uppercase_N)' >> tomato_profile.mak
	echo 'TOMATO_BUILD_USB = "$(USB)"' >> tomato_profile.mak

	echo 'export EXTRACFLAGS := $(EXTRA_CFLAGS) -DBCMWPA2 -DBCMARM -fno-delete-null-pointer-checks -marm -march=armv7-a -mtune=cortex-a9 $(if $(filter $(NVRAM_SIZE),0),,-DCONFIG_NVRAM_SIZE=$(NVRAM_SIZE)) $(EXTRA_64KCFLAG)' >> tomato_profile.mak

# Note that changes to variables in tomato_profile.mak don't
# get propogated to this invocation of make!
	@echo ""
	@echo "Using $(N) profile, $(B) build config."
	@echo ""

	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp -f config_current .config

	@cd router/busybox && \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@cd router ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@$(MAKE) -C router oldconfig
	@cp -f $(LINUXDIR)/.config $(LINUXDIR)/config_current
	@cp -f router/busybox/.config router/busybox/config_current
	@cp -f router/.config router/config_current

include help.mak

help: help_main
	@echo ""
	@echo "..etc.. - other build configs"
	@echo " clean          -C router clean"
	@echo " cleanimage     rm -rf image"
	@echo " cleantools     clean btools, mksquashfs"
	@echo " cleankernel    -C Linux distclean (but preserves .config)"
	@echo " distclean      distclean of Linux & busybox (but preserve .configs)"
	@echo " prepk          -C Linux oldconfig dep"

.PHONY: all clean distclean cleanimage cleantools cleankernel prepk what setprofile help
.PHONY: a b c d m nc Makefile allversions tomato_profile.mak
