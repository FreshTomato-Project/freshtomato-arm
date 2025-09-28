# Broadcom Linux Router Makefile
#
# Copyright 2005, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
#


include common.mak

define patch_files
 find patches/$(1) -maxdepth 1 -type f -name '*.patch' | sort -t '\0' -n | while read FILE; do \
  ( if patch -p0 -N -s --dry-run < $$FILE 2>/dev/null; then \
   patch -p0 -N -r - --no-backup-if-mismatch < $$FILE; \
  fi ) \
 done
endef

define unpatch_files
 find patches/$(1) -maxdepth 1 -type f -name '*.patch' | sort -t '\0' -n -r | while read FILE; do \
  ( if patch -p0 -Rf --dry-run --silent < $$FILE 2>/dev/null; then \
   patch -p0 -R -N -E -r - --no-backup-if-mismatch < $$FILE; \
  fi ) \
 done
endef

define CMAKE_CrossOptions
 ( \
  echo "SET(CMAKE_CROSSCOMPILING \"TRUE\")" >>$(1); \
  echo "SET(TOP $(TOP))" >>$(1); \
  echo "SET(CMAKE_SYSTEM_NAME Linux)" >>$(1); \
  echo "SET(CMAKE_SYSTEM_VERSION $(LINUX_KERNEL))" >>$(1); \
  echo "SET(CMAKE_SYSTEM $(PLATFORM))" >>$(1); \
  echo "SET(CMAKE_SYSTEM_PROCESSOR $(ARCH))" >>$(1); \
  echo "SET(CMAKE_C_COMPILER $(CC))" >>$(1); \
  echo "SET(CMAKE_CXX_COMPILER $(CXX))" >>$(1); \
  echo "SET(CMAKE_C_COMPILER_RANLIB $(RANLIB))" >>$(1); \
  echo "SET(CMAKE_AR $(AR))" >>$(1); \
  echo "SET(CMAKE_LINKER $(LD))" >>$(1); \
  echo "SET(CMAKE_NM $(NM))" >>$(1); \
  echo "SET(CMAKE_OBJCOPY $(OBJCOPY))" >>$(1); \
  echo "SET(CMAKE_OBJDUMP $(OBJDUMP))" >>$(1); \
  echo "SET(CMAKE_RANLIB $(RANLIB))" >>$(1); \
  echo "SET(CMAKE_STRIP $(STRIP))" >>$(1); \
  echo "SET(CMAKE_FIND_ROOT_PATH $(TOOLCHAIN))" >>$(1); \
  echo "SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" >>$(1); \
  echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)" >>$(1); \
  echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)" >>$(1); \
 )
endef

define meson_CrossOptions
 ( \
  echo "[host_machine]" >>$(1); \
  echo "system = 'linux'" >>$(1); \
  echo "cpu_family = '$(if $(TCONFIG_BCMARM),arm,mips)'" >>$(1); \
  echo "cpu = '$(if $(TCONFIG_BCMARM),arm7-a,$(if $(TCONFIG_MIPSR2),mips32r2,mips32))'" >>$(1); \
  echo "endian = 'little'" >>$(1); \
  echo "[binaries]" >>$(1); \
  echo "c = '$(CC)'" >>$(1); \
  echo "cpp = '$(CXX)'" >>$(1); \
  echo "ar = '$(AR)'" >>$(1); \
  echo "strip = '$(STRIP)'" >>$(1); \
  echo "pkg-config = '/usr/bin/pkg-config'" >>$(1); \
  echo "[built-in options]" >>$(1); \
  echo "wrap_mode = 'nodownload'" >>$(1); \
  echo "[properties]" >>$(1); \
  echo "have_c99_vsnprintf=true" >>$(1); \
  echo "have_c99_snprintf=true" >>$(1); \
  echo "have_unix98_printf=true" >>$(1); \
 )
endef


#
#
#


SEP=`$(eval progress=$(shell echo $$(($(progress)+1))))` \
printf "\n\033[41;1m   $@ ${progress}$\/${totalSteps} \033[0m\033]2;Building $@ ${progress}$\/${totalSteps}\007\n"

export PARALLEL_BUILD := -j$(shell grep -c '^processor' /proc/cpuinfo)
export HAVE_TOMATO := y
comma := ,

export INSTALL_ZLIB := $(if $(or $(TCONFIG_DNSCRYPT),$(TCONFIG_NFS),$(TCONFIG_SAMBASRV),$(TCONFIG_BBT),$(TCONFIG_MEDIA_SERVER),$(TCONFIG_TOR),$(TCONFIG_TINC),$(TCONFIG_NGINX),$(TCONFIG_BCMARM),$(TCONFIG_ZFS),$(TCONFIG_IRQBALANCE)),y)

ifeq ($(TCONFIG_BCMWL6),y)
 export CFLAGS += -DTRAFFIC_MGMT
 export CFLAGS += -DTRAFFIC_MGMT_RSSI_POLICY
endif

ifeq ($(TCONFIG_BCMARM),y)
 export CFLAGS += -DBCMWPA2
 ifeq ($(TCONFIG_BCMWL6),y)
  export CFLAGS += -DBCMQOS
  export CFLAGS += -DBCM_DCS
  export CFLAGS += -DEXT_ACS
  export CFLAGS += -DD11AC_IOTYPES
  export CFLAGS += -DNAS_GTK_PER_STA
  export CFLAGS += -DPHYMON
  export CFLAGS += -DPROXYARP
  export CONFIG_MFP=y
  export CFLAGS += -DMFP
  export CFLAGS += -D__CONFIG_MFP__
 endif
endif # TCONFIG_BCMARM

export CFLAGS += $(EXTRACFLAGS)
ifeq ($(TCONFIG_BCMBSD),y)
 export CFLAGS += -DBCM_BSD
endif

ifeq ($(TCONFIG_USB_EXTRAS),y)
 NEED_EX_USB = y
endif

ifeq ($(TCONFIG_MICROSD),y)
 NEED_SD_MODULES = y
endif

ifeq ($(TCONFIG_IPSEC),y)
 export TCONFIG_IPSEC := y
else
 TCONFIG_IPSEC :=
endif

ifeq ($(TCONFIG_RAID),y)
 export TCONFIG_RAID := y
else
 TCONFIG_RAID :=
endif

# Specify iptables/iproute2/ipset/pcre/php target
ifeq ($(TCONFIG_BCMARM),y)
 export IPTABLES_TARGET := iptables-1.8.x
 export IPROUTE_TARGET := iproute2-3.x
 export IPSET_TARGET := ipset
 export PCRE_TARGET := pcre2
 export PHP_TARGET := php
else
 export IPTABLES_TARGET := iptables
 export IPROUTE_TARGET := iproute2
 export IPSET_TARGET := ipset-6.24
 export PCRE_TARGET := pcre
 export PHP_TARGET := php7
endif

# IPv6
ifeq ($(TCONFIG_IPV6),y)
 export TCONFIG_IPV6 := y
else
 TCONFIG_IPV6 :=
endif

# different optimization flags
ifeq ($(TCONFIG_OPTIMIZE_SIZE),y)
 OPTSIZE_FLAG :=
else
 export OPTSIZE_FLAG := -DNO_OPTSIZE
endif

ifeq ($(TCONFIG_OPTIMIZE_SIZE_MORE),y)
 OPTSIZE_MORE_FLAG :=
else
 export OPTSIZE_MORE_FLAG := -DNO_OPTSIZE
endif

ifneq ($(TCONFIG_BCMARM),y)
 ifeq ($(TCONFIG_AIO),y)
  export CFLAG_OPTIMIZE = -O3
 else
  export CFLAG_OPTIMIZE = -Os
 endif
endif

# ffmpeg stuff
FFMPEG_FILTER_CONFIG= $(foreach c, $(2), --$(1)="$(c)")

FFMPEG_DEMUXERS := avi flac matroska mov mp3 mpegps mpegts mpegvideo ogg
FFMPEG_DECODERS := aac ac3 flac h264 hevc jpegls mp2 mp3 mpeg1video mpeg2video mpeg4 mpegvideo png vc1 vorbis wmav1 wmav2
FFMPEG_PROTOCOLS := file

FFMPEG_CONFIGURE_DEMUXERS := $(call FFMPEG_FILTER_CONFIG,enable-demuxer,$(FFMPEG_DEMUXERS))
FFMPEG_CONFIGURE_PARSERS :=
FFMPEG_CONFIGURE_DECODERS := $(call FFMPEG_FILTER_CONFIG,enable-decoder,$(FFMPEG_DECODERS))
FFMPEG_CONFIGURE_PROTOCOLS := $(call FFMPEG_FILTER_CONFIG,enable-protocol,$(FFMPEG_PROTOCOLS))

# wolfssl options
WOLFSSL_FLAGS := -fomit-frame-pointer -DFP_MAX_BITS=8192
WOLFSSL_OPTIONS :=
ifeq ($(TCONFIG_WOLFSSLMIN),y)
 WOLFSSL_FLAGS   += -DUSE_SLOW_SHA -DUSE_SLOW_SHA256 -DUSE_SLOW_SHA512 -DWOLFSSL_AES_NO_UNROLL -DWOLFSSL_AES_SMALL_TABLES -DRSA_LOW_MEM -DCURVE25519_SMALL -DED25519_SMALL -DWOLFSSL_SMALL_CERT_VERIFY
 WOLFSSL_OPTIONS += --enable-opensslextra --enable-stunnel $(if $(TCONFIG_OPENVPN),--enable-openvpn,--disable-poly1305 --disable-chacha --disable-md5 --disable-tls13 $(if $(TCONFIG_BBT),,--disable-sha)) --enable-session-ticket \
                    --disable-oldtls --enable-aesgcm=small --enable-certgen --enable-fastmath \
                    --disable-examples --disable-sslv3 --disable-dtls
else
 WOLFSSL_FLAGS   += -DWOLFSSL_ALT_NAMES
 WOLFSSL_OPTIONS += --enable-opensslextra --enable-opensslall --enable-stunnel $(if $(TCONFIG_OPENVPN),--enable-openvpn,) $(if $(TCONFIG_NGINX),--enable-nginx,) --enable-session-ticket \
                    --enable-sni --enable-altcertchains --enable-certgen \
                    --enable-curve25519 --enable-tlsv10 --enable-wpas --enable-fortress --enable-fastmath \
                    --disable-examples --disable-sslv3
endif
ifeq ($(TOMATO_EXPERIMENTAL),1)
 WOLFSSL_OPTIONS += --enable-crypttests --enable-debug
else
 WOLFSSL_OPTIONS += --disable-crypttests --disable-errorstrings
endif
ifeq ($(TCONFIG_BBT),)
 WOLFSSL_OPTIONS += --disable-arc4
endif

# openssl options
OPENSSL_OPTIONS := no-tests no-unit-test no-sse2 no-ssl3 no-comp no-gost no-afalgeng no-devcryptoeng no-nextprotoneg no-capieng no-blake2 no-idea no-mdc2 no-ocb no-seed no-whirlpool no-srp no-cms no-ec2m
ifneq ($(or $(TCONFIG_OPENSSL30),$(TCONFIG_OPENSSL11)),y)
 OPENSSL_OPTIONS +=  no-npn
endif
ifeq ($(TCONFIG_OPENSSL30),)
 OPENSSL_OPTIONS += no-ssl2 no-heartbeats
else
 OPENSSL_OPTIONS += no-crypto-mdebug no-fuzz-libfuzzer no-fuzz-afl no-fips no-fips-securitychecks no-padlockeng
endif
# remove when built without KEYGEN
ifeq ($(TCONFIG_KEYGEN),)
 OPENSSL_OPTIONS += no-camellia no-rmd160 no-rfc3779 no-md2 no-zlib no-ssl-trace no-psk no-ui-console no-filenames no-dso
 ifeq ($(TCONFIG_OPENSSL30),)
  OPENSSL_OPTIONS += no-hw
 endif
 ifeq ($(or $(TCONFIG_OPENSSL30),$(TCONFIG_OPENSSL11)),y)
  OPENSSL_OPTIONS += no-engine no-rc2 no-cast no-md4 no-cmac no-scrypt no-siphash no-rc5 no-dtls no-dtls1 no-dtls1_2 \
                     no-ts no-ocsp no-multiblock no-ct no-sctp no-ubsan no-rdrand no-dgram
 else
  OPENSSL_OPTIONS += no-engines no-sha0 no-smime no-krb5 no-ripemd no-gms no-gmp no-jpake no-libunbound no-store no-dtls1 enable-rc5
 endif # TCONFIG_OPENSSL30 || TCONFIG_OPENSSL11
endif # !TCONFIG_KEYGEN
# build with err only for beta
ifneq ($(TOMATO_EXPERIMENTAL),1)
 OPENSSL_OPTIONS += no-err
endif
ifeq ($(TCONFIG_BBT),)
 OPENSSL_OPTIONS += no-rc4
endif

# set gcc mode for packages that don't compile in default one
ifeq ($(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),)
 GCC_CMODE := -std=gnu99
 GCC_CXXMODE := -std=gnu++98
else
 GCC_CMODE :=
 GCC_CXXMODE := -std=gnu++14
endif

export EXTRA_CXXFLAGS := $(filter-out -D%,$(EXTRACFLAGS))


#
# packages
#


obj-y				+= $(if $(TCONFIG_BCMARM),libbcm,lzma-loader) busybox shared nvram$(BCMEX) $(if $(TCONFIG_BCMARM),eapd$(BCMEX)/linux,et) \
				   libbcmcrypto wlconf$(BCMEX) $(if $(TCONFIG_BCMARM),nas$(BCMEX)) prebuilt
obj-$(TCONFIG_EMF)		+= emf$(BCMEX) igs$(BCMEX)
obj-$(TCONFIG_DPSTA)		+= dpsta
obj-$(TCONFIG_DHDAP)		+= $(if $(TCONFIG_BCM714),dhd$(BCMEX),dhd) $(if $(TCONFIG_BCM714),,$(if $(TCONFIG_BCM7),pciefd))

ifeq ($(TCONFIG_BCMARM),y)
 ifeq ($(CONFIG_BCMWL6),y)
  ifeq ($(TCONFIG_TUXERA),y)
   obj-y			+= tuxera
  else
   obj-$(TCONFIG_NTFS)		+= $(if $(TCONFIG_UFSDA),ufsd-asus)
  endif
 endif
endif

ifeq ($(CONFIG_BCMWL6),y)
 obj-$(TCONFIG_NTFS)		+= $(if $(TCONFIG_UFSD),ufsd,ntfs-3g)
else
 obj-$(TCONFIG_NTFS)		+= ntfs-3g
endif

obj-$(TCONFIG_BCMARM)		+= taskset
obj-y				+= rc wanuptime rom others www bridge etc pppd rp-pppoe utils$(BCMEX) cstats rstats hotplug2 udevtrigger libusb10 usbmodeswitch
obj-y				+= $(if $(TCONFIG_BCMARM),libmnl) $(IPSET_TARGET)
obj-$(TCONFIG_PPTPD)		+= pptpd
obj-$(TCONFIG_PROXY)		+= igmpproxy udpxy
obj-$(TCONFIG_IPERF)		+= iperf
obj-$(TCONFIG_BCMBSD)		+= bsd
obj-$(TCONFIG_HAVEGED)		+= haveged

ifneq ($(TCONFIG_HTTPS),)
 ifeq ($(TCONFIG_WOLFSSL),y)
  export SSL_TARGET = wolfssl
 else
  ifeq ($(TCONFIG_OPENSSL30),y)
   export OPENSSLDIR = openssl-3.0
  else
   ifeq ($(TCONFIG_OPENSSL11),y)
    export OPENSSLDIR = openssl-1.1
   else
    export OPENSSLDIR = openssl
   endif
  endif
  export SSL_TARGET = $(OPENSSLDIR)
 endif
 obj-y				+= $(SSL_TARGET)
else
 obj-y				+= cyassl
endif

ifeq ($(TCONFIG_OPENVPN),y)
 obj-y				+= $(SSL_TARGET)
 ifneq ($(TCONFIG_OPTIMIZE_SIZE_MORE),y)
  obj-y				+= lz4
  export INSTALL_LZ4 := y
 endif
 ifeq ($(TCONFIG_BCMARM),y)
  obj-y				+= libcap-ng openvpn
 else
  obj-y				+= openvpn-2.5
 endif
 obj-y				+= openvpn_plugin_auth_nvram
endif

obj-y				+= $(SSL_TARGET) mssl httpd dropbear
obj-y				+= $(if $(or $(TCONFIG_BCMARM),$(TCONFIG_BBT),$(TCONFIG_NGINX)),zlib libcurl,) mdu
obj-y				+= $(if $(TCONFIG_DNSSEC),$(if $(TCONFIG_HTTPS),gmp nettle)) dnsmasq
obj-$(TCONFIG_DNSCRYPT)		+= zlib libsodium dnscrypt
obj-$(TCONFIG_STUBBY)		+= $(SSL_TARGET) libyaml getdns
obj-$(TCONFIG_FTP)		+= $(if $(TCONFIG_FTP_SSL),$(SSL_TARGET)) vsftpd
obj-$(TCONFIG_SDHC)		+= mmc
obj-$(TCONFIG_L2TP)		+= xl2tpd
obj-$(TCONFIG_RAID)		+= mdadm
obj-$(TCONFIG_NFS)		+= e2fsprogs zlib libevent portmap libnfsidmap nfs-utils
obj-$(TCONFIG_USB)		+= e2fsprogs p910nd comgt libjson-c libubox uqmi sd-idle apcupsd
obj-$(TCONFIG_NOCAT)		+= $(if $(TCONFIG_BCMARM),zlib libffi libiconv gettext-tiny $(PCRE_TARGET) glib2,glib) nocat
obj-$(TCONFIG_SAMBASRV)		+= $(if $(or $(TCONFIG_BCMARM),$(TCONFIG_NGINX)),libiconv,) zlib samba3 wsdd2
obj-$(TCONFIG_IPV6)		+= dhcpv6
obj-$(TCONFIG_BBT)		+= zlib libevent $(SSL_TARGET) libcurl transmission
obj-$(TCONFIG_MEDIA_SERVER)	+= zlib sqlite $(if $(TCONFIG_BCMARM),libiconv) libogg flac libjpeg-turbo libexif libid3tag libvorbis ffmpeg minidlna
obj-$(TCONFIG_CONNTRACK_TOOL)	+= libmnl libnfnetlink libnetfilter_conntrack libnetfilter_log libnetfilter_queue conntrack-tools
obj-y				+= $(IPTABLES_TARGET) $(IPROUTE_TARGET) # iptables <- conntrack-tools for ARM!
obj-y				+= $(IPTABLES_TARGET) miniupnpd # iptables <- conntrack-tools for ARM!
obj-$(TCONFIG_EBTABLES)		+= ebtables
obj-$(TCONFIG_NANO)		+= libncurses nano
obj-$(TCONFIG_ZEBRA)		+= zebra
obj-$(TCONFIG_SNMP)		+= snmp
obj-$(TCONFIG_WIREGUARD)	+= wireguard-tools
obj-$(TCONFIG_HFS)		+= $(if $(TCONFIG_BCMARM),$(SSL_TARGET) diskdev_cmds-332.25)
obj-$(TCONFIG_TOR)		+= zlib libevent $(SSL_TARGET) tor
obj-$(TCONFIG_TINC)		+= zlib $(SSL_TARGET) lzo lz4 tinc
obj-$(TCONFIG_NGINX)		+= zlib $(SSL_TARGET) sqlite libcurl $(if $(TCONFIG_BCMARM),,spawn-fcgi) $(PCRE_TARGET) libncurses $(if $(TCONFIG_BCMARM),,libatomic_ops) \
				   libiconv libxml2 libpng libjpeg-turbo mysql libzip $(if $(TCONFIG_BCMARM),libffi gettext-tiny glib2) $(PHP_TARGET) nginx
obj-$(TCONFIG_MDNS)		+= libdaemon expat avahi
obj-$(TCONFIG_ZFS)		+= zlib $(SSL_TARGET) libiconv gettext-tiny util-linux zfs
obj-$(TCONFIG_IRQBALANCE)	+= zlib libffi libiconv gettext-tiny $(PCRE_TARGET) glib2 irqbalance


#
#
#


obj-clean := $(foreach obj, $(obj-y) $(obj-n) $(obj-), $(obj)-clean)
obj-install := $(foreach obj,$(obj-y),$(obj)-install)

export PLATFORM LIBDIR USRLIBDIR

ifeq ($(TCONFIG_BCMARM),y)
 LINUX_VERSION = 2_6_36
 LINUX_KERNEL = 2.6.36
 export LINUX_VERSION

 ifeq ($(TCONFIG_BCM714),y)
  export BCMSRC = src-rt-7.14.114.x/src

  ifeq ($(TCONFIG_DHDAP),y)
   export CONFIG_DHDAP		 = y
   export CFLAGS		+= -D__CONFIG_DHDAP__
   export SRCBASE_DHD		:= $(SRCBASE)/../dhd/src
   export SRCBASE_DHD24		:= $(SRCBASE)/../dhd24/src
   export SRCBASE_FW		:= $(SRCBASE)/../4365/src
   export SRCBASE_SYS		:= $(SRCBASE_DHD)
   include Makefile.fw
  endif # TCONFIG_DHDAP

  ifeq ($(TCONFIG_GMAC3),y)
   export CFLAGS += -D__CONFIG_GMAC3__
  endif

 else ifeq ($(TCONFIG_BCM7),y)
  export BCMSRC = src-rt-7.x.main/src

  ifeq ($(TCONFIG_DHDAP),y)
   export CONFIG_DHDAP		 = y
   export CFLAGS		+= -D__CONFIG_DHDAP__
   export DHDAP_USE_SEPARATE_CHECKOUTS := 1
   export SRCBASE_DHD		:= $(SRCBASE)/router
   export SRCBASE_FW		:= $(SRCBASE)/../../43602/src
   PCIEFD_TARGETS_LIST		:= 43602a1-roml

   ifeq ($(WLTEST),1)
    PCIEFD_TARGET_NAME := pcie-ag-splitrx-fdap-mbss-mfgtest-seqcmds-phydbg-txbf-pktctx-amsdutx-ampduretry-chkd2hdma
   else
    PCIEFD_TARGET_NAME := pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus
   endif

   PCIEFD_EMBED_HEADER_TEMPLATE	:= $(SRCBASE_DHD)/shared/rtecdc_router.h.in
   PCIEFD_EMBED_HEADER		:= $(SRCBASE_DHD)/shared/rtecdc_router.h
   obj-pciefd			:= $(patsubst %,%-obj,$(PCIEFD_TARGETS_LIST))
   install-pciefd		:= $(patsubst %,%-install,$(PCIEFD_TARGETS_LIST))
  endif # TCONFIG_DHDAP

  ifeq ($(TCONFIG_GMAC3),y)
   export CFLAGS += -D__CONFIG_GMAC3__
  endif

 else # SDK6
  export BCMSRC = src-rt-6.x.4708
 endif # TCONFIG_BCM714

 WLAN_ComponentsInUse := bcmwifi clm ppr olpc
 include $(SRCBASE)/makefiles/WLAN_Common.mk
 export BASEDIR := $(WLAN_TreeBaseA)
 export EXTRALIBS = -lgcc_s
 ifeq ($(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),)
  export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3/lib
 else
  ifeq ($(CONFIG_TOOLCHAIN73),y)
   export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-uclibc-7.3/usr/lib
  else
   export LD_LIBRARY_PATH := $(SRCBASE)/toolchains/hndtools-arm-uclibc-5.3/usr/lib
  endif
 endif
 ifeq (2_6_36,$(LINUX_VERSION))
  ifeq ($(TCONFIG_BCM7),y)
   export LINUXDIR := $(SRCBASE)/linux/linux-2.6.36
  else
   export LINUXDIR := $(BASEDIR)/src-rt-6.x.4708/linux/linux-2.6.36
  endif # TCONFIG_BCM7
  export KBUILD_VERBOSE := 1
  export BUILD_MFG := 0
 endif # LINUX_VERSION

 SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)
 SUBMAKE_SETTINGS += ARCH=$(ARCH)
 export CFLAGS += -O2
 export OPTCFLAGS = -O2
 WLCFGDIR=$(SRCBASE)/wl/config

 ifeq ($(TCONFIG_EMF),y)
  export CFLAGS += -D__CONFIG_EMF__
 endif

 ROOT_IMG := target.squashfs
 CRAMFSDIR := cramfs

 export MKSYM :=

 obj-prelibs = $(filter nvram$(BCMEX) libbcmcrypto shared netconf libupnp libz libbcm, $(obj-y))
 obj-postlibs := $(filter-out $(obj-prelibs), $(obj-y))

 ifeq ($(TCONFIG_BCMWL6),y)
  include ../../$(SRCBASE)/makefiles/WLAN_Common.mk
 endif
endif # TCONFIG_BCMARM


#
# Basic rules
#


all: countSteps clean-build libc $(obj-y) $(if $(TCONFIG_BCM7),version,) kernel

countSteps:
	@totalSteps=0
	@progress=0
	@$(foreach n, $(obj-y), $(eval totalSteps=$(shell echo $$(($(totalSteps)+1)))))
	@echo ${totalSteps}

ifeq ($(TCONFIG_BCM7),y)
version: $(SRCBASE)/include/epivers.h

$(SRCBASE)/include/epivers.h:
	$(MAKE) -C $(SRCBASE)/include
ifeq ($(TCONFIG_DHDAP),y)
	$(MAKE) -C $(SRCBASE_DHD)/include
	$(MAKE) -C $(SRCBASE_FW)/include
endif
endif

kernel: $(LINUXDIR)/.config
	@$(SEP)

ifneq ($(TCONFIG_BCMARM),y)
	@if ! grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) zImage CC=$(KERNELCC) $(PARALLEL_BUILD); \
	fi
	if grep -q "CONFIG_MODULES=y" $(LINUXDIR)/.config ; then \
		$(MAKE) -C $(LINUXDIR) modules CC=$(KERNELCC) $(PARALLEL_BUILD); \
	fi
	$(MAKE) -C $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed srctree=$(LINUXDIR) TCONFIG_MIPSR2=$(TCONFIG_MIPSR2) $(PARALLEL_BUILD)
else # TCONFIG_BCMARM
	$(MAKE) compressed-clean
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) |\
		$(MAKE) $(PARALLEL_BUILD) -C ${LINUXDIR} -f - $(SUBMAKE_SETTINGS) zImage
	+$(MAKE) CONFIG_SQUASHFS=$(CONFIG_SQUASHFS) -C $(SRCBASE)/router/compressed ARCH=$(ARCH)

	@$(SEP)
	$(if $(shell grep "CONFIG_MODULES=y" ${LINUXDIR}/.config), \
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) | $(MAKE) $(PARALLEL_BUILD) -C ${LINUXDIR} -f - $(SUBMAKE_SETTINGS) MFG_WAR=1 zImage ; \
	(echo '.NOTPARALLEL:' ; cat ${LINUXDIR}/Makefile) | $(MAKE) $(PARALLEL_BUILD) -C ${LINUXDIR} -f - ARCH=$(ARCH) modules)
	# Preserve the debug versions of these and strip for release
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/vmlinux)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/wl/wl.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/et/et.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/ctf/ctf.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/bcm57xx/bcm57xx.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/emf/emf.ko)
	$(call STRIP_DEBUG_SYMBOLS,$(LINUXDIR)/drivers/net/igs/igs.ko)
endif # !TCONFIG_BCMARM

ifneq ($(TCONFIG_BCMARM),y)
lzma-loader:
	@$(SEP)
	$(MAKE) -C $(SRCBASE)/lzma-loader CROSS_COMPILE=$(CROSS_COMPILE) TCONFIG_MIPSR2=$(TCONFIG_MIPSR2) $(PARALLEL_BUILD)

lzma-loader-install:
endif # !TCONFIG_BCMARM

kmod: dummy
	@$(SEP)
	$(MAKE) -C $(LINUXDIR) modules CC=$(KERNELCC) $(PARALLEL_BUILD)

testfind:
	cd $(TARGETDIR)/lib/modules/* && find -name "*.o" -exec mv -i {} . \; || true
	cd $(TARGETDIR)/lib/modules/* && find -type d -delete || true

countInstallSteps:
	@totalSteps=0
	@progress=0
	@$(foreach n, $(obj-install), $(eval totalSteps=$(shell echo $$(($(totalSteps)+1)))))
	@echo ${totalSteps}

install package: countInstallSteps $(obj-install) $(LINUXDIR)/.config
	@printf "\n\033[41;1m   Installing \033[0m\033]2;Installing\007\n"
	install -d $(TARGETDIR)

# kernel modules
	$(MAKE) -C $(LINUXDIR) modules_install \
		INSTALL_MOD_STRIP="--strip-debug -x -R .comment -R .note -R .pdr -R .mdebug.abi32 -R .note.gnu.build-id -R .gnu.attributes -R .reginfo" \
		DEPMOD=/bin/true INSTALL_MOD_PATH=$(TARGETDIR)

ifneq ($(TCONFIG_BCMARM),y)
 ifneq ($(CONFIG_BCMWL6)$(TCONFIG_BLINK),)
  ifneq ($(TCONFIG_USBAP),y)
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv diag/* . && rm -rf diag
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv et.4702/* . && rm -rf et.4702 || true
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv usb/* . && rm -rf usb
  endif
 endif
else
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv diag/* . && rm -rf diag
	-cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv et.4702/* . && rm -rf et.4702 || true
endif # !TCONFIG_BCMARM
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv et/* . && rm -rf et || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv wl/* . && rm -rf wl || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv cifs/* . && rm -rf cifs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jffs2/* . && rm -rf jffs2 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jffs/* . && rm -rf jffs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv zlib_inflate/* . && rm -rf zlib_inflate || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv zlib_deflate/* . && rm -rf zlib_deflate || true
	cd $(TARGETDIR)/lib/modules/*/kernel/lib && mv lzo/* . && rm -rf lzo || true
	rm -rf $(TARGETDIR)/lib/modules/*/pcmcia

	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ext2/* . && rm -rf ext2 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ext3/* . && rm -rf ext3 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jbd/* . && rm -rf jbd || true
ifeq ($(TCONFIG_BCMARM),y)
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ext4/* . && rm -rf ext4 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jbd2/* . && rm -rf jbd2 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv exfat/* . && rm -rf exfat || true
endif
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv fat/* . && rm -rf fat || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv jfs/* . && rm -rf jfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv vfat/* . && rm -rf vfat || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv msdos/* . && rm -rf msdos || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv fuse/* . && rm -rf fuse || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv ntfs/* . && rm -rf ntfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv smbfs/* . && rm -rf smbfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv reiserfs/* . && rm -rf reiserfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv hfs/* . && rm -rf hfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv hfsplus/* . && rm -rf hfsplus || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv lockd/* . && rm -rf lockd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nfsd/* . && rm -rf nfsd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nfs/* . && rm -rf nfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv xfs/* . && rm -rf xfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv nls/* . && rm -rf nls || true
	cd $(TARGETDIR)/lib/modules/*/kernel/fs && mv exportfs/* . && rm -rf exportfs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/net && mv sunrpc/* . && rm -rf sunrpc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/net && mv auth_gss/* . && rm -rf auth_gss || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound/core && mv oss/* . && rm -rf oss || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound/core && mv seq/* . && rm -rf seq || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound && mv core/* . && rm -rf core || true
	cd $(TARGETDIR)/lib/modules/*/kernel/sound && mv usb/* . && rm -rf usb || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv hcd/* . && rm -rf hcd || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv host/* . && rm -rf host || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv storage/* . && rm -rf storage || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv serial/* . && rm -rf serial || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv core/* . && rm -rf core || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv class/* . && rm -rf class || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv misc/* . && rm -rf misc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/usb && mv usbip/* . && rm -rf usbip || true
ifneq ($(CONFIG_BCMWL6)$(TCONFIG_BLINK),)
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/mmc && mv core/* . && rm -rf core || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/mmc && mv card/* . && rm -rf card || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/mmc && mv host/* . && rm -rf host || true
endif
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/hid && mv usbhid/* . && rm -rf usbhid || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv joystick/* . && rm -rf joystick || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv keyboard/* . && rm -rf keyboard || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv misc/* . && rm -rf misc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/input && mv mouse/* . && rm -rf mouse || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video && mv uvc/* . && rm -rf uvc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video && mv pwc/* . && rm -rf pwc || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video/gspca && mv gl860/* . && rm -rf gl860 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video/gspca && mv m5602/* . && rm -rf m5602 || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video/gspca && mv stv06xx/* . && rm -rf stv06xx || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media/video && mv gspca/* . && rm -rf gspca || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/media && mv video/* . && rm -rf video || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv bcm57xx/* . && rm -rf bcm57xx || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv emf/* . && rm -rf emf || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv igs/* . && rm -rf igs || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv ctf/* . && rm -rf ctf || true
	cd $(TARGETDIR)/lib/modules/*/kernel/drivers/net && mv usb/* . && rm -rf usb || true
	cd $(TARGETDIR)/lib/modules && rm -f */source || true

# misc
	for dir in $(wildcard $(patsubst %,$(INSTALLDIR)/%,$(obj-y))) ; do \
		(cd $${dir} && tar cpf - .) | (cd $(TARGETDIR) && tar xpf -) \
	done

ifneq ($(TCONFIG_L7),y)
	rm -f $(TARGETDIR)/usr/lib/*tables/libipt_layer7.so
endif
ifeq ($(TCONFIG_TUXERA),y)
	cd $(TARGETDIR)/usr/sbin && ln -sf ntfsck fsck.ntfs
endif

# uClibc
	install $(LIBDIR)/ld-uClibc.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libcrypt.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libpthread.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libgcc_s.so.1 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/libgcc_s.so.1
	install $(LIBDIR)/libc.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libdl.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libm.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libnsl.so.0 $(TARGETDIR)/lib/
	install $(LIBDIR)/libutil.so.0 $(TARGETDIR)/lib/
ifneq ($(TCONFIG_BCMARM),y)
	install $(LIBDIR)/librt-0.9.30.1.so $(TARGETDIR)/lib/librt.so.0
else
	install $(LIBDIR)/librt.so.0 $(TARGETDIR)/lib/librt.so.0
endif
ifneq ($(TCONFIG_NGINX)$(TCONFIG_NANO)$(TCONFIG_BBT),)
 ifneq ($(TCONFIG_BCMARM),y)
	install $(LIBDIR)/libstdc++.so.6 $(TARGETDIR)/lib/libstdc++.so.6
 else
	install $(LIBDIR)/../../lib/libstdc++.so.6 $(TARGETDIR)/lib/libstdc++.so.6
 endif # TCONFIG_BCMARM
	cd $(TARGETDIR)/lib && ln -sf libstdc++.so.6 libstdc++.so
	$(STRIP) $(TARGETDIR)/lib/libstdc++.so.6
endif # TCONFIG_NGINX TCONFIG_NANO
ifneq ($(TCONFIG_OPTIMIZE_SHARED_LIBS),y)
	install $(LIBDIR)/libresolv.so.0 $(TARGETDIR)/lib/
	$(STRIP) $(TARGETDIR)/lib/*.so.0
endif

	@cd $(TARGETDIR) && $(TOP)/others/rootprep.sh

ifneq ($(TCONFIG_BBT)$(TCONFIG_SAMBASRV),yy)
	@cd $(TARGETDIR) && $(TOP)/others/rootprep.sh ln_usr_share
else
 ifeq ($(TCONFIG_TOR),y)
	@cd $(TARGETDIR) && $(TOP)/others/rootprep.sh ln_tor_geoip
 endif
endif

	@echo ---

ifeq ($(TCONFIG_OPTIMIZE_SHARED_LIBS),y)
	@$(SRCBASE)/btools/libfoo.pl
else
	@$(SRCBASE)/btools/libfoo.pl --noopt
endif
	@chmod 0555 $(TARGETDIR)/lib/*.so*
	@chmod 0555 $(TARGETDIR)/usr/lib/*.so*

# moved to run after libfoo.pl - to make sure shared libs include all symbols needed by extras
# separated/copied extra stuff
	@rm -rf $(PLATFORMDIR)/extras
	@mkdir $(PLATFORMDIR)/extras
	@mkdir $(PLATFORMDIR)/extras/ipsec
	@mkdir $(PLATFORMDIR)/extras/raid
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/ip_gre.*o $(PLATFORMDIR)/extras/ || true
#	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/usr/lib/*tables/libipt_policy.*o $(PLATFORMDIR)/extras/ipsec/ || true

	$(if $(TCONFIG_OPENVPN),@cp -f,$(if $(TCONFIG_USB_EXTRAS),@cp -f,$(if $(TCONFIG_IPV6),@cp -f,@mv))) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/tun.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_EBTABLES),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/bridge/netfilter/ebt*.*o $(PLATFORMDIR)/extras/ || true

	$(if $(TCONFIG_RAID),@cp -f,@mv) $(TARGETDIR)/usr/sbin/mdadm $(PLATFORMDIR)/extras/raid/ || true
	$(if $(TCONFIG_RAID),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/md/*.ko $(PLATFORMDIR)/extras/raid/ || true
	$(if $(TCONFIG_RAID),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/drivers/md || true

	@cp $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/netfilter/ip_set*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ifb.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/sched/sch_red.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/ntfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/smbfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/reiserfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/jfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nfsd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/lockd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/exportfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_NFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/sunrpc.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/auth_rpcgss.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/net/rpcsec_gss_krb5.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/fs/xfs.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi/sr_mod.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/scanner.*o $(PLATFORMDIR)/extras/ || true

	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbserial.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/option.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/sierra.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/cdc-acm.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/mii.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/cdc_*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usbnet.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/huawei_ether.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/rndis_host.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/cdc-wdm.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/qmi_wwan.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ftdi_sio.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/pl2303.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_SD_MODULES),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/mmc/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(NEED_SD_MODULES),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/drivers/mmc || true

	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ch341.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbip*.*o $(PLATFORMDIR)/extras/ || true
ifeq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_UPS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbkbd.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_UPS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/hid*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/usbmouse.*o $(PLATFORMDIR)/extras/ || true
else
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/hid/usbkbd.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/hid/usbmouse.*o $(PLATFORMDIR)/extras/ || true
endif
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ipw.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/audio.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/ov51*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/pwc*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/emi*.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/cdc_subset.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ipheth.*o $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/net/usb || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/media/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/media || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/sound/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/sound || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/sound/* $(PLATFORMDIR)/extras/ || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/sound || true
ifneq ($(TCONFIG_BCMARM),y)
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/input/evdev.*o $(PLATFORMDIR)/extras/ || true
endif
	$(if $(TCONFIG_UPS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/input/* $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_UPS),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/drivers/input || true
endif
	$(if $(TCONFIG_UPS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/hid/* $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_UPS),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/drivers/hid || true
endif
	@cp -f $(TARGETDIR)/lib/modules/*/kernel/drivers/net/bcm57*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_CTF),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ctf*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_PPTP),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/pptp.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_L2TP),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/net/pppol2tp.*o $(PLATFORMDIR)/extras/ || true
	@mv $(TARGETDIR)/lib/modules/*/kernel/drivers/net/ppp_deflate.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/crypto/*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/crypto || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/xfrm/*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/net/xfrm || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/key/*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@ls,@rm -rf) $(TARGETDIR)/lib/modules/*/kernel/net/key || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv*/xfrm*.ko $(PLATFORMDIR)/extras/ipsec/ || true
#	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv*/tunnel*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv*/ah*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv*/esp*.ko $(PLATFORMDIR)/extras/ipsec/ || true
	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv*/ipcomp*.ko $(PLATFORMDIR)/extras/ipsec/ || true
#	$(if $(TCONFIG_IPSEC),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/netfilter/xt_policy.ko $(PLATFORMDIR)/extras/ipsec/ || true

	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_cp9*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_cp1251.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_euc-jp.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_sjis.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_gb2312.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_euc-kr.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_SAMBASRV),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_big5.*o $(PLATFORMDIR)/extras/ || true

ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_USB),@cp -f,$(if $(TCONFIG_SDHC),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/fs/nls_*.*o $(PLATFORMDIR)/extras/ || true
endif
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/usb/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/cdrom/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi/*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/leds/*.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_USB),@cp -f,$(if $(TCONFIG_SDHC),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/fs/ext2.*o $(PLATFORMDIR)/extras/ || true
endif
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/ext3.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_USB),@cp -f,$(if $(TCONFIG_SDHC),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/fs/jbd.*o $(PLATFORMDIR)/extras/ || true
endif
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/mbcache.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_BCMARM),y)
	$(if $(TCONFIG_USB),@cp -f,$(if $(TCONFIG_SDHC),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/fs/fat.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB),@cp -f,$(if $(TCONFIG_SDHC),@cp -f,@mv)) $(TARGETDIR)/lib/modules/*/kernel/fs/vfat.*o $(PLATFORMDIR)/extras/ || true
else
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/exfat.*o $(PLATFORMDIR)/extras/ || true
endif
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/msdos.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/fuse.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_HFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/hfs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_HFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/hfsplus.*o $(PLATFORMDIR)/extras/ || true

ifneq ($(TCONFIG_USB),y)
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/usb || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/scsi || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/leds || true
endif

	$(if $(TCONFIG_USB_EXTRAS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/connector/cn.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_USB_EXTRAS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/drivers/block/loop.*o $(PLATFORMDIR)/extras/ || true
ifneq ($(TCONFIG_USB_EXTRAS),y)
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/connector || true
	@rm -rf $(TARGETDIR)/lib/modules/*/kernel/drivers/block || true
endif
	$(if $(TCONFIG_CIFS),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/cifs.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(TCONFIG_BRCM_NAND_JFFS2),$(TCONFIG_JFFS2)),$(if $(TCONFIG_JFFSV1),@mv,@cp -f),@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/jffs2.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(TCONFIG_BRCM_NAND_JFFS2),$(TCONFIG_JFFS2)),$(if $(TCONFIG_JFFSV1),@mv,@cp -f),@mv) $(TARGETDIR)/lib/modules/*/kernel/lib/zlib_*.*o $(PLATFORMDIR)/extras/ || true
	$(if $(or $(TCONFIG_BRCM_NAND_JFFS2),$(TCONFIG_JFFS2)),$(if $(TCONFIG_JFFSV1),@cp -f,@mv),@mv) $(TARGETDIR)/lib/modules/*/kernel/fs/jffs.*o $(PLATFORMDIR)/extras/ || true
	[ ! -f $(TARGETDIR)/lib/modules/*/kernel/lib/* ] && rm -rf $(TARGETDIR)/lib/modules/*/kernel/lib || true
	$(if $(TCONFIG_L7),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/ipv4/netfilter/ipt_layer7.*o $(PLATFORMDIR)/extras/ || true
	$(if $(TCONFIG_L7),@cp -f,@mv) $(TARGETDIR)/lib/modules/*/kernel/net/netfilter/xt_layer7.*o $(PLATFORMDIR)/extras/ || true

	@mkdir -p $(PLATFORMDIR)/extras/apps
	@mkdir -p $(PLATFORMDIR)/extras/lib

	@mv $(TARGETDIR)/usr/sbin/ttcp $(PLATFORMDIR)/extras/apps/ || true
	@mv $(TARGETDIR)/usr/sbin/mii-tool $(PLATFORMDIR)/extras/apps/ || true
	@cp -r $(TARGETDIR)/usr/sbin/robocfg $(PLATFORMDIR)/extras/apps/ || true

	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/usr/lib/libusb* $(PLATFORMDIR)/extras/lib/ || true
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/usr/sbin/usb_modeswitch $(PLATFORMDIR)/extras/apps/ || true
	@cp usbmodeswitch/usb_modeswitch.conf $(PLATFORMDIR)/extras/apps/usb_modeswitch.conf || true
	@cp usbmodeswitch/usb_modeswitch.setup $(PLATFORMDIR)/extras/apps/usb_modeswitch.setup || true
	@mkdir -p $(PLATFORMDIR)/extras/apps/usb_modeswitch.d
	@cp -f usbmodeswitch/data/usb_modeswitch.d/* $(PLATFORMDIR)/extras/apps/usb_modeswitch.d || true
ifneq ($(NEED_EX_USB),y)
	@rm -rf $(TARGETDIR)/rom/etc/usb_modeswitch.d || true
	@rm -f  $(TARGETDIR)/rom/etc/usb_modeswitch.conf || true
endif
	$(if $(NEED_EX_USB),@cp -f,@mv) $(TARGETDIR)/usr/sbin/chat $(PLATFORMDIR)/extras/apps/ || true

	@mkdir -p $(TARGETDIR)/rom/etc/l7-protocols
ifeq ($(TCONFIG_L7PAT),y)
	@cd layer7 && ./squish.sh
	cp layer7/squished/*.pat $(TARGETDIR)/rom/etc/l7-protocols
endif

	busybox/examples/depmod.pl -k $(LINUXDIR)/vmlinux -b $(TARGETDIR)/lib/modules/*/
#	@mv $(TARGETDIR)/lib/modules/*/modules.dep $(TARGETDIR)/lib/modules/
	@echo ---

	@rm -f $(TARGETDIR)/lib/modules/*/build

ifneq ($(TCONFIG_BCMARM),y)
	$(MAKE) -C $(LINUXDIR)/scripts/squashfs mksquashfs-lzma
	$(LINUXDIR)/scripts/squashfs/mksquashfs-lzma $(TARGETDIR) $(PLATFORMDIR)/target.image -all-root -noappend -no-duplicates | tee target.info
else
image:
	$(MAKE) -C squashfs-4.2 mksquashfs
	squashfs-4.2/mksquashfs $(TARGETDIR) $(PLATFORMDIR)/$(ROOT_IMG) -noappend -all-root
endif

#	Package kernel and filesystem
#	if grep -q "CONFIG_EMBEDDED_RAMDISK=y" $(LINUXDIR)/.config ; then \
#	    cp $(PLATFORMDIR)/target.image $(LINUXDIR)/arch/mips/ramdisk/$${CONFIG_EMBEDDED_RAMDISK_IMAGE} ; \
#	    $(MAKE) -C $(LINUXDIR) zImage ; \
#	else \
#	    cp $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/vmlinuz $(PLATFORMDIR)/ ; \
#	    trx -o $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.image ; \
#	fi

# 	Pad self-booting Linux to a 64 KB boundary
#	cp $(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/zImage $(PLATFORMDIR)/
#	dd conv=sync bs=64k < $(PLATFORMDIR)/zImage > $(PLATFORMDIR)/linux.bin
# 	Append filesystem to self-booting Linux
#	cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/linux.bin

libc:	$(LIBDIR)/ld-uClibc.so.0
#	$(MAKE) -C ../../../tools-src/uClibc all
#	$(MAKE) -C ../../../tools-src/uClibc install


#
# cleaners
#


clean: clean-build $(obj-clean)
	@rm -rf layer7/squished
	@rm -f .ipv6-y .ipv6-n
	@make -C config clean

clean-build: dummy
	@rm -rf $(TARGETDIR)
	@rm -rf $(INSTALLDIR)
	@rm -f $(PLATFORMDIR)/linux.trx $(PLATFORMDIR)/vmlinuz $(PLATFORMDIR)/target.image
	@rm -rf $(PLATFORMDIR)/extras

distclean: clean
ifneq ($(INSIDE_MAK),1)
	@$(MAKE) -C $(SRCBASE) $@ INSIDE_MAK=1
endif
#	-rm -f $(LIBDIR)/*.so.0  $(LIBDIR)/*.so


#
# configuration
#


CONFIG_IN := config/config.in

config/conf config/mconf:
	@$(MAKE) -C config

rconf: config/conf
	@config/conf $(CONFIG_IN)

rmconf: config/mconf
	@config/mconf $(CONFIG_IN)

roldconf: config/conf
	@config/conf -o $(CONFIG_IN)
	@$(MAKE) prebuilt-clean libbcmcrypto-clean dhcpv6-clean dnsmasq-clean $(IPROUTE_TARGET)-clean
	@$(MAKE) cstats-clean httpd-clean mdu-clean mssl-clean nvram$(BCMEX)-clean rc-clean rstats-clean shared-clean wanuptime-clean
ifeq ($(TCONFIG_BCMARM),y)
	@$(MAKE) compressed-clean
endif

kconf:
	@$(MAKE) -C $(LINUXDIR) config

kmconf:
	@$(MAKE) -C $(LINUXDIR) menuconfig

koldconf:
	@$(MAKE) -C $(LINUXDIR) oldconfig
	@$(MAKE) -C $(LINUXDIR) include/linux/version.h

bboldconf:
	@$(MAKE) -C busybox oldconfig

config conf: rconf kconf

menuconfig mconf: rmconf kmconf

.ipv6-y .ipv6-n:
	@rm -f .ipv6-y .ipv6-n
	@$(MAKE) $(IPTABLES_TARGET)-clean ebtables-clean pppd-clean $(if $(TCONFIG_BCMARM),,zebra-clean) dnsmasq-clean $(IPROUTE_TARGET)-clean
	@touch $@

dependconf: .ipv6-$(if $(TCONFIG_IPV6),y,n)

oldconfig oldconf: koldconf roldconf dependconf bboldconf


#
# overrides and extra dependencies
#

ifneq ($(TCONFIG_BCM714),y) # only for BCM7
ifeq ($(TCONFIG_BCM7),y)
$(obj-pciefd):
# Build PCIEFD firmware only if it is not prebuilt
ifeq ($(TCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	+$(MAKE) CROSS_COMPILE=arm-none-eabi -C $(SRCBASE_FW)/dongle/rte/wl $(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-roml-obj,%,$@).h ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-roml-obj,%,$@).h $(SRCBASE_DHD)/shared/rtecdc_$(patsubst %-roml-obj,%,$@).h && \
		echo "#include <rtecdc_$(patsubst %-roml-obj,%,$@).h>" >> $(PCIEFD_EMBED_HEADER); \
	fi;
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-ram-obj,%,$@).h ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/rtecdc_$(patsubst %-ram-obj,%,$@).h $(SRCBASE_DHD)/shared/rtecdc_$(patsubst %-ram-obj,%,$@).h && \
		echo "#include <rtecdc_$(patsubst %-ram-obj,%,$@).h>" >> $(PCIEFD_EMBED_HEADER); \
	fi;
	if [ -f $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data.c ]; then \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data.c $(SRCBASE_FW)/wl/clm/src/wlc_clm_data.c.GEN && \
		cp $(SRCBASE_FW)/dongle/rte/wl/builds/$(patsubst %-obj,%,$@)/$(PCIEFD_TARGET_NAME)/wlc_clm_data_inc.c $(SRCBASE_FW)/wl/clm/src/wlc_clm_data_inc.c.GEN; \
	fi;
endif
endif
endif

pciefd-cleangen: pciefd-clean
# Clean PCIEFD firmware only if it is not prebuilt
ifeq ($(TCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	rm -f  $(PCIEFD_EMBED_HEADER)
	cp -f $(PCIEFD_EMBED_HEADER_TEMPLATE) $(PCIEFD_EMBED_HEADER)
endif
endif

pciefd: pciefd-cleangen $(obj-pciefd)

pciefd-clean :
ifeq ($(TCONFIG_DHDAP),y)
ifneq ($(wildcard $(SRCBASE_FW)/wl/sys),)
	+$(MAKE) CROSS_COMPILE=arm-none-eabi -C $(SRCBASE_FW)/dongle/rte/wl clean
	rm -f $(SRCBASE_DHD)/shared/rtecdc*.h
endif
endif

pciefd-install :
	# Nothing to be done here
	@true
endif	# TCONFIG_BCM7

ifeq ($(TCONFIG_DHDAP),y)
dhd:
	@true
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	-$(MAKE) TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRCBASE_DHD)/dhd/exe
endif

dhd-clean:
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	-$(MAKE) TARGET_PREFIX=$(CROSS_COMPILE) -C $(SRCBASE_DHD)/dhd/exe clean
	rm -f $(INSTALLDIR)/dhd/usr/sbin/dhd
	cd $(SRCBASE_DHD)/dhd/exe && rm -f `find ./ -name "*.cmd" && find ./ -name "*.o"`
endif

dhd-install:
ifneq ($(wildcard $(SRCBASE_DHD)/dhd/exe),)
	install -d $(INSTALLDIR)/dhd/usr/sbin
	install $(SRCBASE_DHD)/dhd/exe/dhd $(INSTALLDIR)/dhd/usr/sbin/dhd
	$(STRIP) $(INSTALLDIR)/dhd/usr/sbin/dhd
endif
endif # TCONFIG_DHDAP

busybox: dummy
	@$(SEP)
	$(call patch_files,busybox)
	@$(MAKE) -C $@ EXTRA_CFLAGS="-fPIC $(EXTRACFLAGS)" $(PARALLEL_BUILD)

busybox-install:
	rm -rf $(INSTALLDIR)/busybox
	$(MAKE) -C busybox EXTRA_CFLAGS="-fPIC $(EXTRACFLAGS)" CONFIG_PREFIX=$(INSTALLDIR)/busybox install

busybox-config:
	$(MAKE) -C busybox menuconfig

busybox-clean:
	-@$(MAKE) -C busybox distclean
	$(call unpatch_files,busybox)

www-install:
	@$(MAKE) -C www INSTALLDIR=$(INSTALLDIR)/www TOMATO_EXPERIMENTAL=$(TOMATO_EXPERIMENTAL) install

cyassl/stamp-h1:
	cd cyassl && autoreconf -fsi && \
		CFLAGS="$(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC -DNO_MD4 -DNO_ERROR_STRINGS -DNO_HC128 -DNO_RABBIT -DNO_PSK -DNO_DSA -DNO_DH -DNO_PWDBASED" \
		CXXFLAGS="-Os -Wall -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		PTHREAD_LIBS="-lpthread" \
		$(CONFIGURE)
	@touch $@

cyassl: cyassl/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

cyassl-install:
	@true

cyassl-clean:
	-@$(MAKE) -C cyassl clean
	@rm -f cyassl/stamp-h1

openssl/stamp-h1:
ifeq ($(TCONFIG_BCMARM),)
ifeq ($(TCONFIG_KEYGEN),y)
	@mv patches/openssl/102-tomato-mips-specific.patch patches/openssl/102-tomato-mips-specific.patch.tmp || true
else
	@mv patches/openssl/102-tomato-mips-specific.patch.tmp patches/openssl/102-tomato-mips-specific.patch || true
endif
	$(call patch_files,openssl)
endif # !TCONFIG_BCMARM

	cd openssl && \
		CC=$(CC:$(CROSS_COMPILE)%=%) \
		AR=$(AR:$(CROSS_COMPILE)%=%) \
		NM=$(NM:$(CROSS_COMPILE)%=%) \
		RANLIB=$(RANLIB:$(CROSS_COMPILE)%=%) \
		./Configure $(HOSTCONFIG) $(EXTRACFLAGS) $(if $(TCONFIG_BCMARM),-O3,-Os) --prefix=/usr --openssldir=/etc/ssl -DOPENSSL_NO_BUF_FREELISTS \
			-ffunction-sections -fdata-sections -Wl,--gc-sections -fomit-frame-pointer \
			shared $(OPENSSL_OPTIONS)

ifeq ($(TCONFIG_BCMARM),)
	cd openssl && mkdir -p include/openssl && \
	ln -sf ../../crypto/krb5/krb5_asn.h include/openssl/krb5_asn.h
ifeq ($(TCONFIG_KEYGEN),)
	cd openssl && \
	ln -sf ../../crypto/camellia/camellia.h include/openssl/camellia.h && \
	ln -sf ../../crypto/mdc2/mdc2.h include/openssl/mdc2.h && \
	ln -sf ../../crypto/ripemd/ripemd.h include/openssl/ripemd.h && \
	ln -sf ../../crypto/seed/seed.h include/openssl/seed.h && \
	ln -sf ../../crypto/ts/ts.h include/openssl/ts.h && \
	ln -sf ../../crypto/engine/engine.h include/openssl/engine.h && \
	ln -sf ../../crypto/rsa/rsa.h include/openssl/rsa.h && \
	ln -sf ../../crypto/dh/dh.h include/openssl/dh.h && \
	ln -sf ../../crypto/idea/idea.h include/openssl/idea.h && \
	ln -sf ../../crypto/rc2/rc2.h include/openssl/rc2.h && \
	ln -sf ../../crypto/cast/cast.h include/openssl/cast.h
endif
endif # !TCONFIG_BCMARM

	@$(MAKE) -C openssl depend
	@$(MAKE) -C openssl clean
	@touch $@

openssl: openssl/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ INSTALL_PREFIX=$(TOP)/openssl/staged install_sw

openssl-install:
	install -D openssl/libcrypto.so.1.0.0 $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.1.0.0
	install -D openssl/libssl.so.1.0.0 $(INSTALLDIR)/openssl/usr/lib/libssl.so.1.0.0
	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libcrypto.so.1.0.0
	$(STRIP) $(INSTALLDIR)/openssl/usr/lib/libssl.so.1.0.0
	install -D openssl/apps/openssl $(INSTALLDIR)/openssl/usr/sbin/openssl
	$(STRIP) $(INSTALLDIR)/openssl/usr/sbin/openssl
	chmod 0500 $(INSTALLDIR)/openssl/usr/sbin/openssl
	cd $(INSTALLDIR)/openssl/usr/lib && ln -sf libcrypto.so.1.0.0 libcrypto.so
	cd $(INSTALLDIR)/openssl/usr/lib && ln -sf libssl.so.1.0.0 libssl.so

openssl-clean:
	-@$(MAKE) -C openssl clean
	@rm -f openssl/stamp-h1
	@rm -rf openssl/staged
ifeq ($(TCONFIG_BCMARM),)
	@rm -rf openssl/include
	$(call unpatch_files,openssl)
endif # !TCONFIG_BCMARM

openssl-1.1/stamp-h1:
ifeq ($(TCONFIG_BCMARM),)
ifeq ($(TCONFIG_KEYGEN),y)
	@mv patches/openssl-1.1/104-reduce-size-for-smaller-targets.patch patches/openssl-1.1/104-reduce-size-for-smaller-targets.patch.tmp || true
else
	@mv patches/openssl-1.1/104-reduce-size-for-smaller-targets.patch.tmp patches/openssl-1.1/104-reduce-size-for-smaller-targets.patch || true
endif
endif # !TCONFIG_BCMARM

	$(call patch_files,openssl-1.1)
	cd openssl-1.1 && \
		CC=$(CC:$(CROSS_COMPILE)%=%) \
		AR=$(AR:$(CROSS_COMPILE)%=%) \
		NM=$(NM:$(CROSS_COMPILE)%=%) \
		RANLIB=$(RANLIB:$(CROSS_COMPILE)%=%) \
		./Configure $(HOSTCONFIG)-freshtomato $(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O3) --prefix=/usr --openssldir=/etc/ssl \
			-ffunction-sections -fdata-sections -Wl,--gc-sections \
			shared $(OPENSSL_OPTIONS) --api=1.0.0 \
			no-async no-aria no-sm2 no-sm3 no-sm4 \
			-DOPENSSL_PREFER_CHACHA_OVER_GCM
	@$(MAKE) -C openssl-1.1 clean
	@touch $@

openssl-1.1: openssl-1.1/stamp-h1
	$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/openssl-1.1/staged install_sw

openssl-1.1-install:
	install -D openssl-1.1/libcrypto.so.1.1 $(INSTALLDIR)/openssl-1.1/usr/lib/libcrypto.so.1.1
	install -D openssl-1.1/libssl.so.1.1 $(INSTALLDIR)/openssl-1.1/usr/lib/libssl.so.1.1
	$(STRIP) $(INSTALLDIR)/openssl-1.1/usr/lib/libssl.so.1.1
	$(STRIP) $(INSTALLDIR)/openssl-1.1/usr/lib/libcrypto.so.1.1
	install -D openssl-1.1/apps/openssl $(INSTALLDIR)/openssl-1.1/usr/sbin/openssl11
	$(STRIP) $(INSTALLDIR)/openssl-1.1/usr/sbin/openssl11
	chmod 0500 $(INSTALLDIR)/openssl-1.1/usr/sbin/openssl11
	cd $(INSTALLDIR)/openssl-1.1/usr/lib && ln -sf libcrypto.so.1.1 libcrypto.so
	cd $(INSTALLDIR)/openssl-1.1/usr/lib && ln -sf libssl.so.1.1 libssl.so
	cd $(INSTALLDIR)/openssl-1.1/usr/sbin && ln -sf openssl11 openssl

openssl-1.1-clean:
	-@$(MAKE) -C openssl-1.1 clean
	@rm -f openssl-1.1/stamp-h1
	@rm -rf openssl-1.1/staged
	$(call unpatch_files,openssl-1.1)

openssl-3.0/stamp-h1:
	$(call patch_files,openssl-3.0)
	cd openssl-3.0 && \
		CC=$(CC:$(CROSS_COMPILE)%=%) \
		AR=$(AR:$(CROSS_COMPILE)%=%) \
		NM=$(NM:$(CROSS_COMPILE)%=%) \
		RANLIB=$(RANLIB:$(CROSS_COMPILE)%=%) \
		./Configure $(HOSTCONFIG)-freshtomato $(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O3) --prefix=/usr --openssldir=/etc/ssl \
			-ffunction-sections -fdata-sections -Wl,--gc-sections \
			shared $(OPENSSL_OPTIONS) --api=1.0.0 \
			no-async no-aria no-sm2 no-sm3 no-sm4 no-cmp \
			-DOPENSSL_PREFER_CHACHA_OVER_GCM
	@$(MAKE) -C openssl-3.0 clean
	@touch $@

openssl-3.0: openssl-3.0/stamp-h1
	$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/openssl-3.0/staged install_sw

openssl-3.0-install:
	install -D openssl-3.0/libcrypto.so.3 $(INSTALLDIR)/openssl-3.0/usr/lib/libcrypto.so.3
	install -D openssl-3.0/libssl.so.3 $(INSTALLDIR)/openssl-3.0/usr/lib/libssl.so.3
	$(STRIP) $(INSTALLDIR)/openssl-3.0/usr/lib/libssl.so.3
	$(STRIP) $(INSTALLDIR)/openssl-3.0/usr/lib/libcrypto.so.3
	install -D openssl-3.0/apps/openssl $(INSTALLDIR)/openssl-3.0/usr/sbin/openssl30
	$(STRIP) $(INSTALLDIR)/openssl-3.0/usr/sbin/openssl30
	chmod 0500 $(INSTALLDIR)/openssl-3.0/usr/sbin/openssl30
	cd $(INSTALLDIR)/openssl-3.0/usr/lib && ln -sf libcrypto.so.3 libcrypto.so
	cd $(INSTALLDIR)/openssl-3.0/usr/lib && ln -sf libssl.so.3 libssl.so
	cd $(INSTALLDIR)/openssl-3.0/usr/sbin && ln -sf openssl30 openssl

openssl-3.0-clean:
	-@$(MAKE) -C openssl-3.0 clean
	@rm -f openssl-3.0/stamp-h1
	@rm -rf openssl-3.0/staged
	$(call unpatch_files,openssl-3.0)

wolfssl/stamp-h1:
	$(call patch_files,wolfssl)
	cd wolfssl && ./autogen.sh && \
		CFLAGS="$(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O3) -Wall $(EXTRACFLAGS) -fPIC $(WOLFSSL_FLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --libdir=/usr/lib --enable-static $(WOLFSSL_OPTIONS) $(if $(TCONFIG_BCMARM),,--disable-threadlocal) --disable-dependency-tracking
	@touch $@

wolfssl: wolfssl/stamp-h1
	$(SEP)
	$(MAKE) -C $@
	$(MAKE) -C $@ DESTDIR=$(TOP)/$@/staged install

wolfssl-install:
	install -D wolfssl/src/.libs/libwolfssl.so.42.2.0 $(INSTALLDIR)/wolfssl/usr/lib/libwolfssl.so.42.2.0
	$(STRIP) $(INSTALLDIR)/wolfssl/usr/lib/libwolfssl.so.42.2.0
	cd $(INSTALLDIR)/wolfssl/usr/lib && ln -sf libwolfssl.so.42.2.0 libwolfssl.so.42 && ln -sf libwolfssl.so.42.2.0 libwolfssl.so
	# && ln -sf libwolfssl.so.42.2.0 libcyassl.so
ifeq ($(TOMATO_EXPERIMENTAL),1)
	install -D wolfssl/wolfcrypt/benchmark/.libs/benchmark $(INSTALLDIR)/wolfssl/usr/sbin/benchmark
	$(STRIP) $(INSTALLDIR)/wolfssl/usr/sbin/benchmark
endif

wolfssl-clean:
	-@$(MAKE) -C wolfssl clean
	@rm -f wolfssl/stamp-h1
	@rm -rf wolfssl/staged
	$(call unpatch_files,wolfssl)

bridge/Makefile:
	cd bridge && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix='' --with-linux-headers=$(LINUXDIR)/include

bridge: bridge/Makefile
	@$(SEP)
	@$(MAKE) -C $@

bridge-install:
	install -D bridge/brctl/brctl $(INSTALLDIR)/bridge/usr/sbin/brctl
	$(STRIP) $(INSTALLDIR)/bridge/usr/sbin/brctl

bridge-clean:
	-@$(MAKE) -C bridge clean
	@rm -f bridge/Makefile

gmp/stamp-h1:
	$(call patch_files,gmp)
	cd gmp && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --enable-static --disable-shared --without-readline --disable-fft
	@touch $@

gmp: gmp/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

gmp-install:
	@true

gmp-clean:
	-@$(MAKE) -C gmp clean
	@rm -f gmp/stamp-h1
	$(call unpatch_files,gmp)

nettle/stamp-h1:
	cd nettle && \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O2,-Os) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-I$(TOP)/gmp -fPIC" \
		LDFLAGS="-L$(TOP)/gmp/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=$(TOP)/nettle --enable-mini-gmp --disable-documentation --disable-shared --disable-openssl --disable-fat
	@touch $@

nettle: nettle/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ install

nettle-clean:
	-@$(MAKE) -C nettle clean
	@rm -f nettle/stamp-h1
	@rm -rf nettle/include nettle/lib nettle/bin nettle/share

dnsmasq:
	@$(SEP)
	$(call patch_files,dnsmasq)
	$(MAKE) -C dnsmasq $(PARALLEL_BUILD) \
	COPTS="-DHAVE_BROKEN_RTC -DHAVE_TOMATO -DNO_ID -DNO_GMP -DUSE_IPSET $(if $(TCONFIG_OPTIMIZE_SIZE_MORE),-DNO_LOOP,) $(if $(TCONFIG_BCMARM),,-DNO_INOTIFY -DNO_DUMPFILE) \
		$(if $(TCONFIG_USB_EXTRAS),,-DNO_TFTP -DNO_SCRIPT -DNO_AUTH -DNO_INOTIFY) \
		$(if $(TCONFIG_DNSSEC),$(if $(TCONFIG_HTTPS),-I$(TOP)/nettle/include -I$(TOP)/gmp -DHAVE_DNSSEC -DHAVE_DNSSEC_STATIC -DNO_GOST)) \
		$(if $(TCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6)" \
	CFLAGS="-Os -Wall -ffunction-sections -fdata-sections $(EXTRACFLAGS) $(GCC_CMODE) $(OPTSIZE_MORE_FLAG)" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections \
		$(if $(TCONFIG_DNSSEC),$(if $(TCONFIG_HTTPS),-L$(TOP)/nettle/lib -L$(TOP)/gmp/.libs))" \
	$(if $(TCONFIG_DNSSEC),$(if $(TCONFIG_HTTPS),PKG_CONFIG_PATH="$(TOP)/nettle/lib/pkgconfig"))

dnsmasq-install:
	install -D dnsmasq/src/dnsmasq $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq
	$(STRIP) $(INSTALLDIR)/dnsmasq/usr/sbin/dnsmasq

dnsmasq-clean:
	-@$(MAKE) -C dnsmasq clean
	$(call unpatch_files,dnsmasq)

iptables:
	@$(SEP)
	$(call patch_files,iptables)
	$(MAKE) -C $@ PREFIX=/usr BINDIR=/usr/sbin LIBDIR=/usr/lib KERNEL_DIR=$(LINUXDIR) COPT_FLAGS="-Os $(EXTRACFLAGS) -U CONFIG_NVRAM_SIZE $(OPTSIZE_FLAG)" $(PARALLEL_BUILD)

iptables-install:
	install -D iptables/iptables $(INSTALLDIR)/iptables/usr/sbin/iptables
	cd $(INSTALLDIR)/iptables/usr/sbin && ln -sf iptables iptables-restore && ln -sf iptables iptables-save
	install -d $(INSTALLDIR)/iptables/usr/lib/iptables
	install -D iptables/extensions/*.so $(INSTALLDIR)/iptables/usr/lib/iptables/
	install -D iptables/libiptc.so $(INSTALLDIR)/iptables/usr/lib/libiptc.so
	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/iptables
	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/iptables/*.so
	$(STRIP) $(INSTALLDIR)/iptables/usr/lib/libiptc.so
ifeq ($(TCONFIG_IPV6),y)
	install iptables/ip6tables $(INSTALLDIR)/iptables/usr/sbin/ip6tables
	$(STRIP) $(INSTALLDIR)/iptables/usr/sbin/ip6tables
	cd $(INSTALLDIR)/iptables/usr/sbin && \
		ln -sf ip6tables ip6tables-restore && \
		ln -sf ip6tables ip6tables-save
endif

iptables-clean:
	-@$(MAKE) -C iptables KERNEL_DIR=$(LINUXDIR) clean
	$(call unpatch_files,iptables)

iptables-1.8.x/stamp-h1:
	$(call patch_files,iptables-1.8.x)
	cd iptables-1.8.x && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -D_GNU_SOURCE -U CONFIG_NVRAM_SIZE $(if $(CONFIG_TOOLCHAIN73),-DCONFIG_TOOLCHAIN73,)" \
		CPPFLAGS="-Os -Wall -D_GNU_SOURCE $(if $(TCONFIG_CONNTRACK_TOOL),-I$(TOP)/libnfnetlink/include -I$(TOP)/libnetfilter_conntrack/include,)" \
		LDFLAGS="$(if $(TCONFIG_CONNTRACK_TOOL),-L$(TOP)/libnfnetlink/src/.libs -lnfnetlink -L$(TOP)/libnetfilter_conntrack/src/.libs -Wl$(comma)-rpath$(comma)$(TOP)/libnetfilter_conntrack/src/.libs \
			-Wl$(comma)-rpath$(comma)$(TOP)/libmnl/staged/usr/lib,) -lrt" \
		PKG_CONFIG_PATH="$(if $(TCONFIG_CONNTRACK_TOOL),$(TOP)/libnfnetlink:$(TOP)/libnetfilter_conntrack/staged/usr/lib/pkgconfig,)" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			$(if $(TCONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
			$(if $(TCONFIG_CONNTRACK_TOOL),,--disable-connlabel) \
			--with-kernel=$(LINUXDIR) --with-xt-lock-name=/var/lock/xtables.lock \
			--disable-nftables --disable-dependency-tracking
	@touch $@

iptables-1.8.x: iptables-1.8.x/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

iptables-1.8.x-install:
	install -D iptables-1.8.x/iptables/.libs/xtables-legacy-multi $(INSTALLDIR)/iptables-1.8.x/usr/sbin/xtables-legacy-multi
	cd $(INSTALLDIR)/iptables-1.8.x/usr/sbin && \
		ln -sf xtables-legacy-multi iptables-legacy-restore && \
		ln -sf xtables-legacy-multi iptables-legacy-save && \
		ln -sf xtables-legacy-multi iptables-legacy && \
		ln -sf xtables-legacy-multi iptables-restore && \
		ln -sf xtables-legacy-multi iptables-save && \
		ln -sf xtables-legacy-multi iptables
	install -d $(INSTALLDIR)/iptables-1.8.x/usr/lib/xtables
	install -D iptables-1.8.x/libiptc/.libs/lib*.so $(INSTALLDIR)/iptables-1.8.x/usr/lib/
	cd $(INSTALLDIR)/iptables-1.8.x/usr/lib && \
		ln -sf libip4tc.so libip4tc.so.2 && \
		ln -sf libip4tc.so libip4tc.so.2.0.0 && \
		ln -sf libip6tc.so libip6tc.so.2 && \
		ln -sf libip6tc.so libip6tc.so.2.0.0
	install -D iptables-1.8.x/libxtables/.libs/lib*.so $(INSTALLDIR)/iptables-1.8.x/usr/lib/
	cd $(INSTALLDIR)/iptables-1.8.x/usr/lib && \
		ln -sf libxtables.so libxtables.so.12 && \
		ln -sf libxtables.so libxtables.so.12.2.0
	install -D iptables-1.8.x/extensions/*.so $(INSTALLDIR)/iptables-1.8.x/usr/lib/xtables
ifeq ($(TCONFIG_IPV6),y)
	cd $(INSTALLDIR)/iptables-1.8.x/usr/sbin && \
		ln -sf xtables-legacy-multi ip6tables-legacy-restore && \
		ln -sf xtables-legacy-multi ip6tables-legacy-save && \
		ln -sf xtables-legacy-multi ip6tables-legacy && \
		ln -sf xtables-legacy-multi ip6tables-restore && \
		ln -sf xtables-legacy-multi ip6tables-save && \
		ln -sf xtables-legacy-multi ip6tables
endif
	$(STRIP) $(INSTALLDIR)/iptables-1.8.x/usr/sbin/xtables-legacy-multi
	$(STRIP) $(INSTALLDIR)/iptables-1.8.x/usr/lib/*.so*
	$(STRIP) $(INSTALLDIR)/iptables-1.8.x/usr/lib/xtables/*.so*

iptables-1.8.x-clean:
	-@$(MAKE) -C iptables-1.8.x KERNEL_DIR=$(LINUXDIR) distclean
	@rm -f iptables-1.8.x/stamp-h1
	$(call unpatch_files,iptables-1.8.x)

iproute2:
	@$(SEP)
	$(call patch_files,iproute2)
	@$(MAKE) -C $@ KERNEL_INCLUDE=$(LINUXDIR)/include \
		EXTRACFLAGS="$(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(TCONFIG_IPV6),-DUSE_IPV6,-DNO_IPV6) $(OPTSIZE_FLAG)" \
		PKG_CONFIG_PATH="$(TOP)/$(IPTABLES_TARGET)/iptables" \
		$(PARALLEL_BUILD)

iproute2-install:
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/tc
	install -D iproute2/ip/ip $(INSTALLDIR)/iproute2/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2/usr/sbin/ip

iproute2-clean:
	-@$(MAKE) -C iproute2 clean
	$(call unpatch_files,iproute2)

iproute2-3.x:
	@$(SEP)
	$(call patch_files,iproute2-3.x)
	@$(MAKE) -C $@ KERNEL_INCLUDE=$(LINUXDIR)/include \
		EXTRA_CCOPTS="$(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections" \
		SHARED_LIBS="" \
		PKG_CONFIG_PATH="$(TOP)/$(IPTABLES_TARGET)/iptables" \
		IPT_LIB_DIR="/usr/lib/xtables" \
		$(PARALLEL_BUILD)

iproute2-3.x-install:
	install -D iproute2-3.x/tc/tc $(INSTALLDIR)/iproute2-3.x/usr/sbin/tc
	$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/sbin/tc
	install -D iproute2-3.x/ip/ip $(INSTALLDIR)/iproute2-3.x/usr/sbin/ip
	$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/sbin/ip
	@if [ -e iproute2-3.x/tc/m_xt.so ] ; then \
		install -D iproute2-3.x/tc/m_xt.so $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/m_xt.so ; \
		ln -sf m_xt.so $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/m_ipt.so ; \
		$(STRIP) $(INSTALLDIR)/iproute2-3.x/usr/lib/tc/*.so ; \
	fi

iproute2-3.x-clean:
	-@$(MAKE) -C iproute2-3.x clean
	-rm -f iproute2-3.x/Config
	$(call unpatch_files,iproute2-3.x)

libnfnetlink/stamp-h1:
	cd libnfnetlink $(if $(TCONFIG_BCMARM),&& autoreconf -fsi,) && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --enable-shared --enable-static
	@touch $@

libnfnetlink: libnfnetlink/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	@rm -f libnfnetlink/src/libnfnetlink.la

libnfnetlink-install:
	install -D libnfnetlink/src/.libs/libnfnetlink.so.0.2.0 $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0.2.0
	$(STRIP) -s $(INSTALLDIR)/libnfnetlink/usr/lib/libnfnetlink.so.0.2.0
	cd $(INSTALLDIR)/libnfnetlink/usr/lib/ && \
		ln -sf libnfnetlink.so.0.2.0 libnfnetlink.so.0 && \
		ln -sf libnfnetlink.so.0.2.0 libnfnetlink.so

libnfnetlink-clean:
	-@$(MAKE) -C libnfnetlink distclean
	@rm -f libnfnetlink/stamp-h1

miniupnpd/stamp-h1:
	$(call patch_files,miniupnpd)
	cd miniupnpd && \
		PKG_CONFIG_LIBDIR="" \
		./configure --igd2 --leasefile --vendorcfg --portinuse --disable-pppconn --firewall=iptables --iptablespath=$(TOP)/$(IPTABLES_TARGET) $(if $(TCONFIG_IPV6),--ipv6,)
	@touch $@

miniupnpd: miniupnpd/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD) \
		CFLAGS="-Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections -D_FORTIFY_SOURCE=2 $(if $(TCONFIG_BCMARM),-DBCMARM -fstack-protector)" \
		LDFLAGS="-Wl,--gc-sections -Wl,--as-needed $(if $(TCONFIG_BCMARM),,-Wl$(comma)--allow-multiple-definition)"

miniupnpd-install:
	install -D miniupnpd/miniupnpd $(INSTALLDIR)/miniupnpd/usr/sbin/miniupnpd
	$(STRIP) $(INSTALLDIR)/miniupnpd/usr/sbin/miniupnpd

miniupnpd-clean:
	-@$(MAKE) -C miniupnpd clean
	@rm -f miniupnpd/config.h
	@rm -f miniupnpd/stamp-h1
	$(call unpatch_files,miniupnpd)

vsftpd:
	@$(SEP)
	$(call patch_files,vsftpd)
	$(MAKE) -C vsftpd $(PARALLEL_BUILD)

vsftpd-install:
	install -D vsftpd/vsftpd $(INSTALLDIR)/vsftpd/usr/sbin/vsftpd
	$(STRIP) -s $(INSTALLDIR)/vsftpd/usr/sbin/vsftpd

vsftpd-clean:
	$(call unpatch_files,vsftpd)

ufsd:
	@$(MAKE) -C $@ all

ufsd-install: ufsd
	@$(MAKE) -C ufsd install INSTALLDIR=$(INSTALLDIR)/ufsd

ntfs-3g/Makefile:
	$(call patch_files,ntfs-3g)
	cd ntfs-3g && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC $(OPTSIZE_FLAG)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) $(if $(TCONFIG_BCMARM),,ac_cv_func_utimensat=no) --enable-shared=no --enable-static=no \
			--disable-library --disable-ldconfig --disable-mount-helper --with-fuse=internal \
			--disable-ntfsprogs --disable-crypto --without-uuid \
			--disable-posix-acls --disable-nfconv --disable-dependency-tracking

ntfs-3g: ntfs-3g/Makefile
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

ntfs-3g-install:
	install -D ntfs-3g/src/ntfs-3g $(INSTALLDIR)/ntfs-3g/bin/ntfs-3g
	$(STRIP) -s $(INSTALLDIR)/ntfs-3g/bin/ntfs-3g
	install -d $(INSTALLDIR)/ntfs-3g/sbin && cd $(INSTALLDIR)/ntfs-3g/sbin && \
		ln -sf ../bin/ntfs-3g mount.ntfs-3g && \
		ln -sf ../bin/ntfs-3g mount.ntfs

ntfs-3g-clean:
	-@$(MAKE) -C ntfs-3g clean
	@rm -f ntfs-3g/Makefile
	$(call unpatch_files,ntfs-3g)

libusb10/Makefile: libusb10/Makefile.in
	$(call patch_files,libusb10)
	cd libusb10 && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		LIBS="-lpthread" \
		$(CONFIGURE) --prefix=/usr --disable-udev ac_cv_lib_rt_clock_gettime=no

libusb10: libusb10/Makefile
	@$(SEP)
	$(MAKE) -C $@

libusb10-install:
	install -D libusb10/libusb/.libs/libusb-1.0.so $(INSTALLDIR)/libusb10/usr/lib/libusb-1.0.so
	$(STRIP) $(INSTALLDIR)/libusb10/usr/lib/*.so
	cd $(INSTALLDIR)/libusb10/usr/lib && ln -sf libusb-1.0.so libusb-1.0.so.0

libusb10-clean:
	-@$(MAKE) -C $@ clean
	@rm -rf libusb10/Makefile
	$(call unpatch_files,libusb10)

usbmodeswitch:
	@$(SEP)
	$(MAKE) -C $@ \
		CFLAGS="-Os $(EXTRACFLAGS) -DLIBUSB10 -Wl,-R/lib:/usr/lib:/opt/usr/lib -I$(TOP)/libusb10/libusb" \
		LDFLAGS="-L$(TOP)/libusb10/libusb/.libs" \
		LIBS="-lpthread -lusb-1.0"
	# install db
	@mkdir -p $(TARGETDIR)/rom/etc/usb_modeswitch.d
	# compress whitespace
	@for D in $(wildcard $(TOP)/usbmodeswitch/data/usb_modeswitch.d/*); do \
		F=`basename $$D`; \
		sed 's/###.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' < $$D > $(TARGETDIR)/rom/etc/usb_modeswitch.d/$$F; \
	done

usbmodeswitch-install:
	install -D usbmodeswitch/usb_modeswitch $(INSTALLDIR)/usbmodeswitch/usr/sbin/usb_modeswitch
	$(STRIP) -s $(INSTALLDIR)/usbmodeswitch/usr/sbin/usb_modeswitch
	@mkdir -p $(TARGETDIR)/rom/etc
	@sed 's/#.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' < $(TOP)/usbmodeswitch/usb_modeswitch.conf > $(TARGETDIR)/rom/etc/usb_modeswitch.conf

dhcpv6/stamp-h1:
	@cd dhcpv6 && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -D_GNU_SOURCE -ffunction-sections -fdata-sections -DTOMATO -I$(SRCBASE)/include -I$(TOP)/shared \
			$(if $(TCONFIG_BCMARM),-DHAVE_CLOEXEC,) $(if $(or $(TCONFIG_BCMARM),$(TCONFIG_BLINK)),-DNEED_DEBUG,)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections $(if $(TCONFIG_BCMARM),,-L$(TOP)/shared -lshared) -L$(TOP)/nvram$(BCMEX) -lnvram" \
		ac_cv_func_setpgrp_void=yes \
		$(CONFIGURE) --prefix= --with-localdbdir=/var
	@$(MAKE) -C dhcpv6 clean
	@touch $@

dhcpv6: dhcpv6/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ dhcp6c $(PARALLEL_BUILD)

dhcpv6-install:
	install -D dhcpv6/dhcp6c $(INSTALLDIR)/dhcpv6/usr/sbin/dhcp6c
	$(STRIP) $(INSTALLDIR)/dhcpv6/usr/sbin/dhcp6c

dhcpv6-clean:
	-@$(MAKE) -C dhcpv6 clean
	@rm -f dhcpv6/Makefile dhcpv6/stamp-h1

wsdd2:
	$(call patch_files,wsdd2)
	@$(SEP)
	@$(MAKE) -C $@ CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -ffunction-sections -fdata-sections -DTOMATO -I$(TOP)/shared" LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections"

wsdd2-install:
	install -D wsdd2/wsdd2 $(INSTALLDIR)/wsdd2/usr/sbin/wsdd2
	$(STRIP) $(INSTALLDIR)/wsdd2/usr/sbin/wsdd2

wsdd2-clean:
	-@$(MAKE) -C wsdd2 clean
	$(call unpatch_files,wsdd2)

pppd/Makefile:
	$(call patch_files,pppd)
	# do not use 'gc-sections'!
	cd pppd && autoreconf -fsi && \
		CFLAGS="$(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O2) -Wall $(EXTRACFLAGS) $(OPTSIZE_MORE_FLAG)" \
		CPPFLAGS="$(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O2)" \
		$(CONFIGURE) --prefix=/usr --sysconfdir=/tmp --with-plugin-dir=/usr/lib/pppd \
			--with-runtime-dir=/var/run --with-logfile-dir=/var/log \
			--with-system-ca-path=/etc/ssl/certs \
			--enable-microsoft-extensions --enable-plugins \
			--disable-openssl-engine --without-openssl \
			--disable-eaptls --disable-peap --disable-systemd \
			--without-pam --without-pcap --without-srp --without-atm \
			--disable-cbcp --disable-mslanman \
			$(if $(TCONFIG_OPTIMIZE_SIZE_MORE),--disable-multilink,--enable-multilink) \
			$(if $(TCONFIG_IPV6),--enable-ipv6cp,--disable-ipv6cp)

pppd: pppd/Makefile
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

pppd-install:
	install -D pppd/pppd/pppd $(INSTALLDIR)/pppd/usr/sbin/pppd
	install -D pppd/chat/chat $(INSTALLDIR)/pppd/usr/sbin/chat
	$(STRIP) $(INSTALLDIR)/pppd/usr/sbin/*
ifeq ($(TCONFIG_PPTP),y)
	install -D pppd/pppd/plugins/pptp/.libs/pptp.so $(INSTALLDIR)/pppd/usr/lib/pppd/pptp.so
endif
ifeq ($(TCONFIG_L2TP),y)
	install -D pppd/pppd/plugins/pppol2tp/.libs/pppol2tp.so $(INSTALLDIR)/pppd/usr/lib/pppd/pppol2tp.so
endif
ifeq ($(or $(TCONFIG_PPTP),$(TCONFIG_L2TP)),y)
	$(STRIP) $(INSTALLDIR)/pppd/usr/lib/pppd/*.so
endif

pppd-clean:
	-@$(MAKE) -C pppd clean
	@rm -f pppd/Makefile
	$(call unpatch_files,pppd)

pptpd/Makefile:
	$(call patch_files,pptpd)
	cd pptpd && \
		CFLAGS="-Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections" \
		$(CONFIGURE) --prefix=$(INSTALLDIR)/pptpd --enable-bcrelay --disable-dependency-tracking

pptpd: pptpd/Makefile
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

pptpd-install:
	@install -D pptpd/pptpd $(INSTALLDIR)/pptpd/usr/sbin/pptpd
	@install -D pptpd/bcrelay $(INSTALLDIR)/pptpd/usr/sbin/bcrelay
	@install -D pptpd/pptpctrl $(INSTALLDIR)/pptpd/usr/sbin/pptpctrl
	@$(STRIP) $(INSTALLDIR)/pptpd/usr/sbin/*

pptpd-clean:
	-@$(MAKE) -C pptpd clean
	@rm -rf pptpd/Makefile
	@rm -f pptpd/plugins/pppd
	$(call unpatch_files,pptpd)

rp-pppoe/src/Makefile:
	$(call patch_files,rp-pppoe)
	cd rp-pppoe/src && \
		CFLAGS="$(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O2) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="$(if $(TCONFIG_OPTIMIZE_SIZE),-Os,-O2) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --enable-plugin=$(TOP)/pppd --disable-debugging \
			ac_cv_linux_kernel_pppoe=yes ac_cv_path_PPPD=$(TOP)/pppd

rp-pppoe: rp-pppoe/src/Makefile
	@$(SEP)
	$(MAKE) -C rp-pppoe/src pppoe-relay rp-pppoe.so $(PARALLEL_BUILD)

rp-pppoe-install:
	install -D rp-pppoe/src/rp-pppoe.so $(INSTALLDIR)/rp-pppoe/usr/lib/pppd/rp-pppoe.so
	$(STRIP) $(INSTALLDIR)/rp-pppoe/usr/lib/pppd/*.so
#	install -D rp-pppoe/src/pppoe-relay $(INSTALLDIR)/rp-pppoe/usr/sbin/pppoe-relay
#	$(STRIP) $(INSTALLDIR)/rp-pppoe/usr/sbin/pppoe-relay

rp-pppoe-clean:
	-@$(MAKE) -C rp-pppoe/src clean
	@rm -f rp-pppoe/src/Makefile
	$(call unpatch_files,rp-pppoe)

zebra/stamp-h1:
	@cd zebra && rm -f config.cache && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --sysconfdir=/etc --enable-netlink $(if $(TCONFIG_IPV6),--enable-ipv6,--disable-ipv6) --disable-ripngd \
			--disable-ospfd --disable-ospf6d --disable-bgpd --disable-bgp-announce --disable-dependency-tracking
	@touch $@

zebra: zebra/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@

zebra-install:
	install -D zebra/zebra/zebra $(INSTALLDIR)/zebra/usr/sbin/zebra
	install -D zebra/ripd/ripd $(INSTALLDIR)/zebra/usr/sbin/ripd
	install -D zebra/lib/libzebra.so $(INSTALLDIR)/zebra/usr/lib/libzebra.so
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/zebra
	$(STRIP) $(INSTALLDIR)/zebra/usr/sbin/ripd
	$(STRIP) $(INSTALLDIR)/zebra/usr/lib/libzebra.so

zebra-clean:
	-@$(MAKE) -C zebra clean
	@rm -f zebra/stamp-h1

xl2tpd:
	@$(SEP)
	$(call patch_files,xl2tpd)
	CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC $(OPTSIZE_FLAG)" \
	LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
	$(MAKE) -C $@ PREFIX=/usr xl2tpd $(PARALLEL_BUILD)

xl2tpd-install:
	install -D xl2tpd/xl2tpd $(INSTALLDIR)/xl2tpd/usr/sbin/xl2tpd
	$(STRIP) $(INSTALLDIR)/xl2tpd/usr/sbin/xl2tpd

xl2tpd-clean:
	-@$(MAKE) -C xl2tpd clean
	$(call unpatch_files,xl2tpd)

libbcm:
	@$(SEP)
	@[ ! -f libbcm/Makefile ] || $(MAKE) -C libbcm

libbcm-install:
	install -D libbcm/libbcm.so $(INSTALLDIR)/libbcm/usr/lib/libbcm.so
	$(STRIP) $(INSTALLDIR)/libbcm/usr/lib/libbcm.so

dropbear/config.h:
	$(call patch_files,dropbear)
	cd dropbear && autoreconf -fsi && ln -sf src/config.guess config.guess && ln -sf src/config.sub config.sub && ln -sf src/install-sh install-sh && \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O3,-Os) -Wall $(EXTRACFLAGS) -DARGTYPE=3 -ffunction-sections -fdata-sections $(OPTSIZE_MORE_FLAG)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		ac_cv_func_logout=no ac_cv_func_logwtmp=no \
		$(CONFIGURE) --disable-zlib --enable-syslog --disable-lastlog --disable-utmp \
			--disable-utmpx --disable-wtmp --disable-wtmpx --disable-pututline --disable-pututxline \
			--disable-loginfunc --disable-pam --enable-openpty --enable-bundled-libtom $(if $(TCONFIG_OPTIMIZE_SIZE_MORE),LTM_CFLAGS=-Os,)
	@$(MAKE) -C dropbear clean

dropbear: dropbear/config.h
	@$(SEP)
ifneq ($(TCONFIG_ZEBRA)$(TCONFIG_BCMARM),)
	$(MAKE) -C $@ PROGRAMS="dropbear dropbearkey scp dbclient" MULTI=1 $(PARALLEL_BUILD)
else
	$(MAKE) -C $@ PROGRAMS="dropbear dropbearkey $(if $(TCONFIG_OPTIMIZE_SIZE),,scp)" MULTI=1 $(PARALLEL_BUILD)
endif

dropbear-install:
	install -D dropbear/dropbearmulti $(INSTALLDIR)/dropbear/usr/bin/dropbearmulti
	$(STRIP) $(INSTALLDIR)/dropbear/usr/bin/dropbearmulti
	cd $(INSTALLDIR)/dropbear/usr/bin && \
	ln -sf dropbearmulti dropbear && \
	ln -sf dropbearmulti dropbearkey && \
	ln -sf dropbearmulti ssh
ifneq ($(TCONFIG_ZEBRA)$(TCONFIG_BCMARM),)
	cd $(INSTALLDIR)/dropbear/usr/bin && \
	ln -sf dropbearmulti dbclient && \
	ln -sf dropbearmulti scp
else
 ifneq ($(TCONFIG_OPTIMIZE_SIZE),y)
	cd $(INSTALLDIR)/dropbear/usr/bin && \
	ln -sf dropbearmulti scp
 endif
endif

dropbear-clean:
	-@$(MAKE) -C dropbear clean
	@rm -f dropbear/config.h
	$(call unpatch_files,dropbear)

zlib/stamp-h1:
	$(call patch_files,zlib)
	cd zlib && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		CPPLAGS="-Os -Wall -fPIC -ffunction-sections -fdata-sections" \
		LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LDSHAREDLIBC="$(EXTRALIBS)" \
		./configure --prefix=/usr --shared --libdir=/usr/lib
	@touch $@

zlib: zlib/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ all $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/zlib/staged install

zlib-install:
ifeq ($(INSTALL_ZLIB),y)
	install -d $(INSTALLDIR)/zlib/usr/lib
	install -D zlib/libz.so.1 $(INSTALLDIR)/zlib/usr/lib/
	$(STRIP) $(INSTALLDIR)/zlib/usr/lib/libz.so.1
endif
	@true

zlib-clean:
	-@$(MAKE) -C zlib clean
	@rm -f zlib/stamp-h1 zlib/Makefile zlib/zconf.h zlib/zlib.pc
	@rm -rf zlib/staged
	$(call unpatch_files,zlib)

sqlite/Makefile:
	cd sqlite && \
		CFLAGS="-Os $(EXTRACFLAGS) -fPIC $(GCC_CMODE) -DHAVE_ISNAN -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
		LDFLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib" \
		$(CONFIGURE) --prefix=/usr --enable-shared --disable-static --enable-threadsafe \
			--disable-editline --disable-readline --disable-session --disable-fts3 \
			--disable-fts4 --disable-fts5 --disable-rtree --soname=legacy

sqlite: sqlite/Makefile
	@$(SEP)
	@$(MAKE) -C $@ all $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/sqlite/staged install

sqlite-install:
ifeq ($(or $(TCONFIG_NGINX),$(TCONFIG_MEDIA_SERVER)),y)
	install -D sqlite/staged/usr/lib/libsqlite3.so.3.50.4 $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.3.50.4
	$(STRIP) $(INSTALLDIR)/sqlite/usr/lib/libsqlite3.so.3.50.4
	cd $(INSTALLDIR)/sqlite/usr/lib/ && \
		ln -sf libsqlite3.so.3.50.4 libsqlite3.so.0 && \
		ln -sf libsqlite3.so.3.50.4 libsqlite3.so
endif
	@true

sqlite-clean:
	-@$(MAKE) -C sqlite clean
	@rm -f sqlite/Makefile
	@rm -rf sqlite/staged

libogg/stamp-h1:
	cd libogg && autoreconf -fsi && \
		CFLAGS="-Os $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix='' --enable-shared --enable-static
	@touch $@

libogg: libogg/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ all $(PARALLEL_BUILD)

libogg-install:
	install -D libogg/src/.libs/libogg.so.0 $(INSTALLDIR)/libogg/usr/lib/libogg.so.0
	$(STRIP) $(INSTALLDIR)/libogg/usr/lib/libogg.so.0

libogg-clean:
	-@$(MAKE) -C libogg clean
	@rm -f libogg/stamp-h1

flac/stamp-h1:
	$(call patch_files,flac)
	cd flac && autoreconf -fsi && \
		CFLAGS="-Os $(EXTRACFLAGS) $(GCC_CMODE) -fPIC -ffunction-sections -fdata-sections" \
		CPPFLAGS="-I$(TOP)/libogg/include -I$(LINUXDIR)/include -fPIC" \
		LDFLAGS="-L$(TOP)/libogg/src/.libs -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LIBS="-logg" \
		$(CONFIGURE) --prefix='' --enable-shared --enable-static --with-ogg=$(TOP)/libogg/src/.libs --with-ogg-includes=$(TOP)/libogg/include/ogg \
			--disable-doxygen-docs --disable-examples --disable-cpplibs \
			--disable-programs --disable-oggtest --disable-thorough-tests \
			--disable-stack-smash-protection --disable-avx --enable-debug=no \
			--without-libiconv-prefix --disable-rpath --disable-dependency-tracking
	@touch $@

flac: flac/stamp-h1
	@$(SEP)
	@$(MAKE) -C flac/src/libFLAC all $(PARALLEL_BUILD)

flac-install:
	install -D flac/src/libFLAC/.libs/libFLAC.so.14.0.0 $(INSTALLDIR)/flac/usr/lib/libFLAC.so.14.0.0
	$(STRIP) $(INSTALLDIR)/flac/usr/lib/libFLAC.so.14.0.0
	cd $(INSTALLDIR)/flac/usr/lib/ && ln -sf libFLAC.so.14.0.0 libFLAC.so.14

flac-clean:
	-@$(MAKE) -C flac clean
	@rm -f flac/stamp-h1
	$(call unpatch_files,flac)

libjpeg-turbo/build/Makefile:
	@rm -rf libjpeg-turbo/build && rm -f libjpeg-turbo/jconfig.h && mkdir -p libjpeg-turbo/build
	cd libjpeg-turbo/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_RPATH=TRUE \
			-DWITH_JPEG8=TRUE \
			-DENABLE_SHARED=TRUE \
			-DENABLE_STATIC=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -fPIC" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -fPIC" \
			-DCMAKE_MODULE_LINKER_FLAGS="-Wl,--gc-sections -fPIC" \
			-DCMAKE_SHARED_LINKER_FLAGS="-Wl,--gc-sections -fPIC" \
			..

libjpeg-turbo: libjpeg-turbo/build/Makefile
	@$(SEP)
	@$(MAKE) -C libjpeg-turbo/build $(PARALLEL_BUILD)
	@$(MAKE) -C libjpeg-turbo/build DESTDIR=$(TOP)/libjpeg-turbo/staged install

libjpeg-turbo-install:
	install -D libjpeg-turbo/build/libjpeg.so.8.3.2 $(INSTALLDIR)/libjpeg-turbo/usr/lib/libjpeg.so.8.3.2
	cd $(INSTALLDIR)/libjpeg-turbo/usr/lib/ && $(STRIP) libjpeg.so.8.3.2 && ln -sf libjpeg.so.8.3.2 libjpeg.so && ln -sf libjpeg.so.8.3.2 libjpeg.so.8
ifeq ($(TCONFIG_NGINX),y)
	install -D libjpeg-turbo/build/libjpeg.so.8.3.2 $(INSTALLDIR)/libjpeg-turbo/usr/lib/libjpeg.so.8.3.2
	cd $(INSTALLDIR)/libjpeg-turbo/usr/lib/ && $(STRIP) libjpeg.so.8.3.2 && ln -sf libjpeg.so.8.3.2 libjpeg.so && ln -sf libjpeg.so.8.3.2 libjpeg.so.8
endif

libjpeg-turbo-clean:
	@rm -rf libjpeg-turbo/build
	@rm -rf libjpeg-turbo/staged
	@rm -f libjpeg-turbo/jconfig.h

libexif/stamp-h1:
	cd libexif && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -fPIC -ffunction-sections -fdata-sections" \
		LDFLAGS="-fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix='' --enable-shared --enable-static \
			--disable-docs --disable-rpath --disable-nls --without-libiconv-prefix --without-libintl-prefix
	@touch $@

libexif: libexif/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ all $(PARALLEL_BUILD)

libexif-install:
	install -D libexif/libexif/.libs/libexif.so.12 $(INSTALLDIR)/libexif/usr/lib/libexif.so.12
	$(STRIP) $(INSTALLDIR)/libexif/usr/lib/libexif.so.12

libexif-clean:
	-@$(MAKE) -C libexif clean
	@rm -f libexif/stamp-h1

libid3tag/static/Makefile:
	@rm -rf libid3tag/static && mkdir -p libid3tag/static
	cd libid3tag/static && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -fPIC -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_CXX_FLAGS="-Wall $(EXTRA_CXXFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-fPIC -Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DBUILD_SHARED_LIBS=OFF \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

libid3tag/build/Makefile:
	@rm -rf libid3tag/build && mkdir -p libid3tag/build
	cd libid3tag/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -fPIC -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_CXX_FLAGS="-Wall $(EXTRA_CXXFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-fPIC -Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

libid3tag: libid3tag/static/Makefile libid3tag/build/Makefile
	@$(SEP)
	$(MAKE) -C libid3tag/static all $(PARALLEL_BUILD)
	$(MAKE) -C libid3tag/build all $(PARALLEL_BUILD)

libid3tag-install:
	install -D libid3tag/build/libid3tag.so.0.16.3 $(INSTALLDIR)/libid3tag/usr/lib/libid3tag.so.0.16.3
	cd $(INSTALLDIR)/libid3tag/usr/lib/ && $(STRIP) libid3tag.so.0.16.3 && ln -sf libid3tag.so.0.16.3 libid3tag.so && ln -sf libid3tag.so.0.16.3 libid3tag.so.0

libid3tag-clean:
	@rm -rf libid3tag/static
	@rm -rf libid3tag/build

libvorbis/stamp-h1:
	cd libvorbis && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		CPPFLAGS="-I$(TOP)/libogg/include -fPIC" \
		LDFLAGS="-L$(TOP)/libogg/src/.libs -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix='' --enable-shared --enable-static --disable-oggtest \
			--with-ogg-includes="$(TOP)/libogg/include" \
			--with-ogg-libraries="$(TOP)/libogg/src/.libs"
	@touch $@

libvorbis: libvorbis/stamp-h1
	@$(SEP)
	@$(MAKE) -C libvorbis/lib all $(PARALLEL_BUILD)

libvorbis-install:
	install -D libvorbis/lib/.libs/libvorbis.so.0 $(INSTALLDIR)/libvorbis/usr/lib/libvorbis.so.0
	$(STRIP) $(INSTALLDIR)/libvorbis/usr/lib/libvorbis.so.0

libvorbis-clean:
	-@$(MAKE) -C libvorbis clean
	@rm -f libvorbis/stamp-h1

ffmpeg/stamp-h1:
	$(call patch_files,ffmpeg)
	cd ffmpeg && \
		./configure --prefix='' --enable-cross-compile --arch=$(ARCH) --target-os=linux \
		--cross-prefix=$(CROSS_COMPILE) --pkg-config="pkg-config" \
		--enable-pthreads --enable-small --enable-shared --enable-gpl \
		--disable-doc --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-ffprobe --disable-symver \
		--disable-postproc --disable-avdevice --disable-swscale --disable-network --disable-avfilter --disable-altivec \
		--disable-muxers --disable-devices --disable-encoders --disable-filters --disable-hwaccels --disable-bsfs \
		--disable-outdevs --disable-armv5te --disable-armv6 --disable-armv6t2 --disable-neon \
		$(if $(TCONFIG_BCMARM),,--disable-mmi --disable-yasm) \
		--disable-demuxers $(FFMPEG_CONFIGURE_DEMUXERS) \
		--disable-decoders $(FFMPEG_CONFIGURE_DECODERS) \
		--disable-parsers $(FFMPEG_CONFIGURE_PARSERS) \
		--disable-protocols $(FFMPEG_CONFIGURE_PROTOCOLS) \
		--extra-cflags="-Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib/staged/usr/include $(OPTSIZE_FLAG)" \
		--extra-ldflags="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -L$(TOP)/zlib/staged/usr/lib" \
		--extra-libs="-lz" \
		--enable-zlib --disable-debug
	@touch $@

ffmpeg: ffmpeg/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ all $(PARALLEL_BUILD)

ffmpeg-install:
	install -D ffmpeg/libavformat/libavformat.so.54 $(INSTALLDIR)/ffmpeg/usr/lib/libavformat.so.54
	install -D ffmpeg/libavcodec/libavcodec.so.54 $(INSTALLDIR)/ffmpeg/usr/lib/libavcodec.so.54
	install -D ffmpeg/libavutil/libavutil.so.51 $(INSTALLDIR)/ffmpeg/usr/lib/libavutil.so.51
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavformat.so.54
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavcodec.so.54
	$(STRIP) $(INSTALLDIR)/ffmpeg/usr/lib/libavutil.so.51

ffmpeg-clean:
	-@$(MAKE) -C ffmpeg clean
	@rm -f ffmpeg/stamp-h1 ffmpeg/config.h ffmpeg/config.mak
	$(call unpatch_files,ffmpeg)

minidlna/stamp-h1:
	$(call patch_files,minidlna)
	cd minidlna && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		CPPFLAGS="-I$(TOP)/shared -I$(TOP)/ffmpeg/libavutil -I$(TOP)/ffmpeg/libavcodec -I$(TOP)/ffmpeg/libavformat -I$(TOP)/ffmpeg -I$(TOP)/flac/include -I$(TOP)/sqlite/staged/usr/include \
			-I$(TOP)/libjpeg-turbo/staged/usr/include -I$(TOP)/libexif -I$(TOP)/libid3tag/build -I$(TOP)/libogg/include -I$(TOP)/libvorbis/include -I$(TOP)/zlib/staged/usr/include" \
		LDFLAGS="-Wl,--gc-sections -ffunction-sections -fdata-sections -L$(TOP)/shared -L$(TOP)/libvorbis/lib/.libs -L$(TOP)/libogg/src/.libs -L$(TOP)/sqlite/staged/usr/lib -L$(TOP)/libexif/libexif/.libs -L$(TOP)/libjpeg-turbo/staged/usr/lib \
			-L$(TOP)/flac/src/libFLAC/.libs -L$(TOP)/libid3tag/build -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/ffmpeg/libavformat -L$(TOP)/ffmpeg/libavcodec -L$(TOP)/ffmpeg/libavutil" \
		LIBS="-lm -lpthread -lgcc_s -lvorbis -lz -logg -lsqlite3 -lexif -ljpeg -lFLAC -lid3tag -lavformat -lavcodec -lavutil" \
		$(CONFIGURE) --prefix=/usr --libdir=/usr/lib --with-db-path=/tmp/minidlna --with-log-path=/var/log --with-os-name="FreshTomato" --with-os-url="https://freshtomato.org/" \
			--with-os-version="Linux/2.6.$(if $(TCONFIG_BCMARM),36.4brcmarm,22.19)" --enable-tivo --disable-nls --disable-dependency-tracking
	@touch $@

minidlna: minidlna/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

minidlna-install:
	install -D minidlna/minidlnad $(INSTALLDIR)/minidlna/usr/sbin/minidlna
	$(STRIP) $(INSTALLDIR)/minidlna/usr/sbin/minidlna

minidlna-clean:
	-@$(MAKE) -C minidlna clean
	@rm -f minidlna/stamp-h1
	$(call unpatch_files,minidlna)

igmpproxy/src/Makefile: igmpproxy/src/Makefile.in
ifneq ($(TCONFIG_BCMARM),y)
	$(call patch_files,igmpproxy)
endif
	cd igmpproxy && autoreconf -fsi && \
		$(CONFIGURE) --prefix=/usr

igmpproxy: igmpproxy/src/Makefile
	@$(SEP)
	@$(MAKE) -C igmpproxy/src \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O3,-Os) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(OPTSIZE_FLAG)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(PARALLEL_BUILD)

igmpproxy-install:
	install -D igmpproxy/src/igmpproxy $(INSTALLDIR)/igmpproxy/usr/sbin/igmpproxy
	$(STRIP) $(INSTALLDIR)/igmpproxy/usr/sbin/igmpproxy

igmpproxy-clean:
	-@$(MAKE) -C igmpproxy/src clean
	@rm -f igmpproxy/src/Makefile
ifneq ($(TCONFIG_BCMARM),y)
	$(call unpatch_files,igmpproxy)
endif

hotplug2:
	@$(SEP)
	$(call patch_files,hotplug2)
	$(MAKE) -C $@ \
		EXTRACFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(CONFIG_TOOLCHAIN53),-DCONFIG_TOOLCHAIN53) $(if $(CONFIG_TOOLCHAIN73),-DCONFIG_TOOLCHAIN73)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections"

hotplug2-install:
	$(MAKE) -C hotplug2 PREFIX=$(INSTALLDIR)/hotplug2 SUBDIRS="" install
	$(MAKE) -C hotplug2/examples PREFIX=$(INSTALLDIR)/hotplug2/rom KERNELVER=$(LINUX_KERNEL) install

hotplug2-clean:
	-@$(MAKE) -C hotplug2
	$(call unpatch_files,hotplug2)

emf$(BCMEX):
	$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/emfconf CROSS=$(CROSS_COMPILE)

emf$(BCMEX)-install:
ifeq ($(TCONFIG_EMF),y)
	install -d $(TARGETDIR)
	$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/emfconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR) install
endif

emf$(BCMEX)-clean:
	-@$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/emfconf clean

igs$(BCMEX):
	$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/igsconf CROSS=$(CROSS_COMPILE)

igs$(BCMEX)-install:
ifeq ($(TCONFIG_EMF),y)
	install -d $(TARGETDIR)
	$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/igsconf CROSS=$(CROSS_COMPILE) INSTALLDIR=$(INSTALLDIR) install
endif

igs$(BCMEX)-clean:
	-@$(MAKE) -C $(if $(TCONFIG_BCMARM),,$(SRCBASE)/)emf$(BCMEX)/igsconf clean

ebtables/stamp-h1: dummy
ifeq ($(TCONFIG_IPV6),y)
	mv patches/ebtables/104-do-not-build-ipv6-extension.patch patches/ebtables/104-do-not-build-ipv6-extension.patch.tmp || true
else
	mv patches/ebtables/104-do-not-build-ipv6-extension.patch.tmp patches/ebtables/104-do-not-build-ipv6-extension.patch || true
endif
	$(call patch_files,ebtables)
	cd ebtables && ./autogen.sh && \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --sysconfdir=/etc --libdir=/usr/lib \
			CFLAGS="-Os $(EXTRACFLAGS) $(if $(TCONFIG_BCMARM),,-DEBT_MIN_ALIGN=4 -D_GNU_SOURCE) -ffunction-sections -fdata-sections -fPIC $(OPTSIZE_FLAG)" \
			LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
			LOCKFILE="/var/lock/ebtables"
	@touch $@

ebtables: ebtables/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@

ebtables-install:
	@mkdir -p $(TARGETDIR)/rom/etc
	@sed 's/#.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' < $(TOP)/ebtables/ethertypes > $(TARGETDIR)/rom/etc/ethertypes
	chmod 0644 $(TARGETDIR)/rom/etc/ethertypes
	install -D ebtables/.libs/ebtables-legacy $(INSTALLDIR)/ebtables/usr/sbin/ebtables-legacy
	install -D ebtables/.libs/libebtc.so.0.0.0 $(INSTALLDIR)/ebtables/usr/lib/libebtc.so.0.0.0
	$(STRIP) $(INSTALLDIR)/ebtables/usr/sbin/ebtables-legacy
	$(STRIP) $(INSTALLDIR)/ebtables/usr/lib/libebtc.so.0.0.0
	cd $(INSTALLDIR)/ebtables/usr/sbin/ && ln -sf ebtables-legacy ebtables
	cd $(INSTALLDIR)/ebtables/usr/lib/ && ln -sf libebtc.so.0.0.0 libebtc.so && ln -sf libebtc.so.0.0.0 libebtc.so.0

ebtables-clean:
	-@$(MAKE) -C ebtables clean
	rm -f ebtables/stamp-h1
	$(call unpatch_files,ebtables)

spawn-fcgi/build:
	$(call patch_files,spawn-fcgi)
	cd spawn-fcgi && \
		$(call meson_CrossOptions, meson-cross.txt) && \
		$(TOP)/meson/meson.py setup build \
		--cross-file meson-cross.txt \
		--prefix=/usr \
		-Dsysconfdir=/etc \
		-Dlocalstatedir=/var \
		-Dc_std=$(if $(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),gnu11,gnu99) \
		-Dc_args="$(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		-Dc_link_args="-Wl,--gc-sections" \
		-Doptimization=s \
		-Dbuildtype=minsize \
		-Ddebug=false \
		-Dwarning_level=1 \
		-Dextra-warnings=false \
		-Dipv6=$(if $(TCONFIG_IPV6),true,false)

spawn-fcgi: spawn-fcgi/build
	@$(SEP)
	$(TOP)/meson/meson.py compile -C $@/build $(PARALLEL_BUILD)

spawn-fcgi-install:
	install -D spawn-fcgi/build/src/spawn-fcgi $(INSTALLDIR)/spawn-fcgi/usr/bin/spawn-fcgi
	$(STRIP) -s $(INSTALLDIR)/spawn-fcgi/usr/bin/spawn-fcgi

spawn-fcgi-clean:
	@rm -rf spawn-fcgi/build
	@rm -f spawn-fcgi/meson-cross.txt
	$(call unpatch_files,spawn-fcgi)

glib/stamp-h1:
ifneq ($(TCONFIG_BCMARM),y)
	$(call patch_files,glib)
endif
	cd glib && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC $(OPTSIZE_FLAG)" \
		LDFLAGS="-Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=/usr --enable-debug=no --enable-static=no --disable-dependency-tracking \
			glib_cv_prog_cc_ansi_proto=no glib_cv_has__inline=yes glib_cv_has__inline__=yes glib_cv_hasinline=yes \
			glib_cv_sane_realloc=yes glib_cv_va_copy=no glib_cv___va_copy=yes glib_cv_va_val_copy=yes glib_cv_rtldglobal_broken=no \
			glib_cv_uscore=no glib_cv_func_pthread_mutex_trylock_posix=yes glib_cv_func_pthread_cond_timedwait_posix=yes glib_cv_sizeof_gmutex=24 \
			glib_cv_byte_contents_gmutex="0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" glib_cv_sys_pthread_getspecific_posix=yes \
			glib_cv_sys_pthread_mutex_trylock_posix=yes glib_cv_sys_pthread_cond_timedwait_posix=yes ac_cv_func_getpwuid_r=yes ac_cv_func_getpwuid_r_posix=yes
	@touch $@

glib: glib/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(INSTALLDIR)/glib install

glib-install:
	@true

glib-clean:
	-@$(MAKE) -C glib clean
	@rm -f glib/stamp-h1
ifneq ($(TCONFIG_BCMARM),y)
	$(call unpatch_files,glib)
endif

nocat/stamp-h1:
	cd nocat && \
		NC_CONF_PATH="/" \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(TCONFIG_BCMARM),-I$(TOP)/glib2/staged/usr/include/glib-2.0 -I$(TOP)/glib2/staged/usr/lib/glib-2.0/include,)" \
		LDFLAGS="-Wl,--gc-sections" \
		LIBS="$(if $(TCONFIG_BCMARM),-L$(TOP)/glib2/staged/usr/lib -L$(TOP)/libiconv/staged/usr/lib -L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -lglib-2.0 -liconv -lpcre2-8,)" \
		$(CONFIGURE) --localstatedir=/var --sysconfdir=/etc --with-firewall=iptables --with-glib-prefix="$(if $(TCONFIG_BCMARM),$(TOP)/glib2/staged,$(INSTALLDIR)/glib/usr)" --disable-dependency-tracking
	@touch $@

nocat: nocat/stamp-h1
	@$(SEP)
ifneq ($(TCONFIG_BCMARM),y)
	@echo --- Integrate glib to nocat installdir ---
	install -D glib/.libs/libglib-1.2.so.0.0.10 $(INSTALLDIR)/nocat/usr/lib/libglib-1.2.so.0.0.10
	cd $(INSTALLDIR)/nocat/usr/lib && ln -s libglib-1.2.so.0.0.10 libglib-1.2.so.0
	$(STRIP) $(INSTALLDIR)/nocat/usr/lib/libglib-1.2.so.0.0.10
endif
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

nocat-install:
	install -D nocat/src/splashd $(INSTALLDIR)/nocat/usr/sbin/splashd
	$(STRIP) $(INSTALLDIR)/nocat/usr/sbin/splashd
	mkdir -p $(INSTALLDIR)/nocat/usr/libexec/nocat
	install -D nocat/libexec/iptables/* $(INSTALLDIR)/nocat/usr/libexec/nocat
	-@rm -rf $(INSTALLDIR)/glib

nocat-clean:
	-@$(MAKE) -C nocat clean
	@rm -f nocat/stamp-h1

pcre/stamp-h1:
	$(call patch_files,pcre)
	cd pcre && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -I$(TOP)/zlib/staged/usr/include -ffunction-sections -fdata-sections" \
		LDFLAGS="-L$(TOP)/zlib/staged/usr/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --disable-cpp --disable-jit --disable-pcregrep-jit --disable-pcretest-libreadline \
			--enable-utf8 --enable-unicode-properties --disable-dependency-tracking
	[ -d pcre/m4 ] || mkdir pcre/m4
	@touch $@

pcre: pcre/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/pcre/staged install

pcre-install:
	install -D pcre/staged/usr/lib/libpcre.so.1.2.13 $(INSTALLDIR)/pcre/usr/lib/libpcre.so.1.2.13
	$(STRIP) -s $(INSTALLDIR)/pcre/usr/lib/libpcre.so.1.2.13
	cd $(INSTALLDIR)/pcre/usr/lib/ && ln -sf libpcre.so.1.2.13 libpcre.so.1 && ln -sf libpcre.so.1.2.13 libpcre.so

pcre-clean:
	-@$(MAKE) -C pcre clean
	@rm -f pcre/stamp-h1
	@rm -rf pcre/staged
	$(call unpatch_files,pcre)

pcre2/build/Makefile:
	@rm -rf pcre2/build && mkdir -p pcre2/build
	cd pcre2/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DBUILD_SHARED_LIBS=ON \
			-DPCRE2_BUILD_PCRE2_8=ON \
			-DPCRE2_BUILD_PCRE2_16=OFF \
			-DPCRE2_BUILD_PCRE2_32=OFF \
			-DPCRE2_DEBUG=OFF \
			-DPCRE2_DISABLE_PERCENT_ZT=ON \
			-DPCRE2_SUPPORT_JIT=OFF \
			-DPCRE2_BUILD_PCRE2GREP=OFF \
			-DPCRE2GREP_SUPPORT_JIT=OFF \
			-DPCRE2GREP_SUPPORT_CALLOUT=OFF \
			-DPCRE2GREP_SUPPORT_CALLOUT_FORK=OFF \
			-DPCRE2_BUILD_TESTS=OFF \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

pcre2: pcre2/build/Makefile
	@$(SEP)
	@$(MAKE) -C pcre2/build all $(PARALLEL_BUILD)
	@$(MAKE) -C pcre2/build DESTDIR=$(TOP)/pcre2/staged install

pcre2-install:
	install -D pcre2/staged/usr/lib/libpcre2-8.so.0.14.0 $(INSTALLDIR)/pcre2/usr/lib/libpcre2-8.so.0.14.0
	$(STRIP) -s $(INSTALLDIR)/pcre2/usr/lib/libpcre2-8.so.0.14.0
	cd $(INSTALLDIR)/pcre2/usr/lib/ && ln -sf libpcre2-8.so.0.14.0 libpcre2-8.so.0 && ln -sf libpcre2-8.so.0.14.0 libpcre2-8.so
ifeq ($(TCONFIG_AIO),y)
	install -D pcre2/staged/usr/lib/libpcre2-posix.so.3.0.6 $(INSTALLDIR)/pcre2/usr/lib/libpcre2-posix.so.3.0.6
	$(STRIP) -s $(INSTALLDIR)/pcre2/usr/lib/libpcre2-posix.so.3.0.6
	cd $(INSTALLDIR)/pcre2/usr/lib/ && ln -sf libpcre2-posix.so.3.0.6 libpcre2-posix.so.3 && ln -sf libpcre2-posix.so.3.0.6 libpcre2-posix.so
endif

pcre2-clean:
	@rm -rf pcre2/build
	@rm -rf pcre2/staged

libxml2/static/Makefile:
	@rm -rf libxml2/static && mkdir -p libxml2/static
	cd libxml2/static && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DBUILD_SHARED_LIBS=OFF \
			-DHAVE_LIBHISTORY=OFF \
			-DHAVE_LIBREADLINE=OFF \
			-DLIBXML2_WITH_C14N=ON \
			-DLIBXML2_WITH_CATALOG=OFF \
			-DLIBXML2_WITH_DEBUG=OFF \
			-DLIBXML2_WITH_FTP=OFF \
			-DLIBXML2_WITH_HTML=ON \
			-DLIBXML2_WITH_HTTP=OFF \
			-DLIBXML2_WITH_ICONV=OFF \
			-DLIBXML2_WITH_ICU=OFF \
			-DLIBXML2_WITH_ISO8859X=OFF \
			-DLIBXML2_WITH_LEGACY=OFF \
			-DLIBXML2_WITH_LZMA=OFF \
			-DLIBXML2_WITH_MODULES=OFF \
			-DLIBXML2_WITH_OUTPUT=ON \
			-DLIBXML2_WITH_PATTERN=ON \
			-DLIBXML2_WITH_PROGRAMS=OFF \
			-DLIBXML2_WITH_PUSH=ON \
			-DLIBXML2_WITH_PYTHON=OFF \
			-DLIBXML2_WITH_READER=ON \
			-DLIBXML2_WITH_REGEXPS=ON \
			-DLIBXML2_WITH_SAX1=ON \
			-DLIBXML2_WITH_SCHEMAS=ON \
			-DLIBXML2_WITH_SCHEMATRON=OFF \
			-DLIBXML2_WITH_TESTS=OFF \
			-DLIBXML2_WITH_THREADS=ON \
			-DLIBXML2_WITH_THREAD_ALLOC=OFF \
			-DLIBXML2_WITH_TREE=ON \
			-DLIBXML2_WITH_VALID=ON \
			-DLIBXML2_WITH_WRITER=ON \
			-DLIBXML2_WITH_XINCLUDE=ON \
			-DLIBXML2_WITH_XPATH=ON \
			-DLIBXML2_WITH_XPTR=ON \
			-DLIBXML2_WITH_XPTR_LOCS=OFF \
			-DLIBXML2_WITH_TLS=O$(if $(TCONFIG_BCMARM),N,FF) \
			-DLIBXML2_WITH_ZLIB=ON \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

libxml2/build/Makefile:
	@rm -rf libxml2/build && mkdir -p libxml2/build
	cd libxml2/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DBUILD_SHARED_LIBS=ON \
			-DHAVE_LIBHISTORY=OFF \
			-DHAVE_LIBREADLINE=OFF \
			-DLIBXML2_WITH_C14N=ON \
			-DLIBXML2_WITH_CATALOG=OFF \
			-DLIBXML2_WITH_DEBUG=OFF \
			-DLIBXML2_WITH_FTP=OFF \
			-DLIBXML2_WITH_HTML=ON \
			-DLIBXML2_WITH_HTTP=OFF \
			-DLIBXML2_WITH_ICONV=OFF \
			-DLIBXML2_WITH_ICU=OFF \
			-DLIBXML2_WITH_ISO8859X=OFF \
			-DLIBXML2_WITH_LEGACY=OFF \
			-DLIBXML2_WITH_LZMA=OFF \
			-DLIBXML2_WITH_MODULES=OFF \
			-DLIBXML2_WITH_OUTPUT=ON \
			-DLIBXML2_WITH_PATTERN=ON \
			-DLIBXML2_WITH_PROGRAMS=OFF \
			-DLIBXML2_WITH_PUSH=ON \
			-DLIBXML2_WITH_PYTHON=OFF \
			-DLIBXML2_WITH_READER=ON \
			-DLIBXML2_WITH_REGEXPS=ON \
			-DLIBXML2_WITH_SAX1=ON \
			-DLIBXML2_WITH_SCHEMAS=ON \
			-DLIBXML2_WITH_SCHEMATRON=OFF \
			-DLIBXML2_WITH_TESTS=OFF \
			-DLIBXML2_WITH_THREADS=ON \
			-DLIBXML2_WITH_THREAD_ALLOC=OFF \
			-DLIBXML2_WITH_TREE=ON \
			-DLIBXML2_WITH_VALID=ON \
			-DLIBXML2_WITH_WRITER=ON \
			-DLIBXML2_WITH_XINCLUDE=ON \
			-DLIBXML2_WITH_XPATH=ON \
			-DLIBXML2_WITH_XPTR=ON \
			-DLIBXML2_WITH_XPTR_LOCS=OFF \
			-DLIBXML2_WITH_TLS=O$(if $(TCONFIG_BCMARM),N,FF) \
			-DLIBXML2_WITH_ZLIB=ON \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

libxml2: libxml2/static/Makefile libxml2/build/Makefile
	@$(SEP)
	@$(MAKE) -C libxml2/static all $(PARALLEL_BUILD)
	@$(MAKE) -C libxml2/build all $(PARALLEL_BUILD)
	@$(MAKE) -C libxml2/static DESTDIR=$(TOP)/libxml2/staged install
	@$(MAKE) -C libxml2/build DESTDIR=$(TOP)/libxml2/staged install

libxml2-install:
ifeq ($(TCONFIG_BCMARM),y)
	install -D libxml2/staged/usr/lib/libxml2.so.16.1.0 $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.16.1.0
	$(STRIP) $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.16.1.0
	cd $(INSTALLDIR)/libxml2/usr/lib && ln -sf libxml2.so.16.1.0 libxml2.so.16 && ln -sf libxml2.so.16.1.0 libxml2.so
else
	install -D libxml2/staged/usr/lib/libxml2.so.2.13.7 $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.2.13.7
	$(STRIP) $(INSTALLDIR)/libxml2/usr/lib/libxml2.so.2.13.7
	cd $(INSTALLDIR)/libxml2/usr/lib && ln -sf libxml2.so.2.13.7 libxml2.so.2 && ln -sf libxml2.so.2.13.7 libxml2.so
endif

libxml2-clean:
	@rm -rf libxml2/static
	@rm -rf libxml2/build
	@rm -rf libxml2/staged

libpng/build/Makefile:
	$(call patch_files,libpng)
	@rm -rf libpng/build && mkdir -p libpng/build
	cd libpng/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -lz" \
			-DPNG_SHARED=on \
			-DPNG_STATIC=on \
			-DCMAKE_POSITION_INDEPENDENT_CODE=on \
			-DPNG_TOOLS=off \
			-DPNG_TESTS=off \
			-DPNG_FRAMEWORK=off \
			-DPNG_HARDWARE_OPTIMIZATIONS=$(if $(TCONFIG_BCMARM),on,off) \
			-DPNG_ARM_NEON=off \
			-Dld-version-script=off \
			-DZLIB_ROOT="$(TOP)/zlib/staged/usr" \
			..

libpng: libpng/build/Makefile
	@$(SEP)
	@$(MAKE) -C libpng/build $(PARALLEL_BUILD)
	@$(MAKE) -C libpng/build DESTDIR=$(TOP)/libpng/staged install

libpng-install:
	install -D libpng/build/libpng16.so.16.50.0 $(INSTALLDIR)/libpng/usr/lib/libpng16.so.16.50.0
	$(STRIP) $(INSTALLDIR)/libpng/usr/lib/libpng16.so.16.50.0
	cd $(INSTALLDIR)/libpng/usr/lib && ln -sf libpng16.so.16.50.0 libpng16.so && ln -sf libpng16.so.16.50.0 libpng16.so.16

libpng-clean:
	@rm -rf libpng/build
	@rm -rf libpng/staged
	$(call unpatch_files,libpng)

libzip/build/Makefile:
	$(call patch_files,libzip)
	@rm -rf libzip/build && mkdir -p libzip/build
	cd libzip/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include -fPIC -pthread" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -fPIC -L$(TOP)/zlib/staged/usr/lib -lz -lpthread" \
			-DENABLE_COMMONCRYPTO=FALSE \
			-DENABLE_GNUTLS=FALSE \
			-DENABLE_OPENSSL=FALSE \
			-DENABLE_MBEDTLS=FALSE \
			-DENABLE_BZIP2=FALSE \
			-DENABLE_LZMA=FALSE \
			-DENABLE_ZSTD=FALSE \
			-DENABLE_WINDOWS_CRYPTO=FALSE \
			-DBUILD_TOOLS=FALSE \
			-DBUILD_REGRESS=FALSE \
			-DBUILD_EXAMPLES=FALSE \
			-DBUILD_DOC=FALSE \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			..

libzip: libzip/build/Makefile
	@$(SEP)
	@$(MAKE) -C libzip/build all $(PARALLEL_BUILD)
	@$(MAKE) -C libzip/build DESTDIR=$(TOP)/libzip/staged install

libzip-install:
	install -D libzip/build/lib/libzip.so.5.5 $(INSTALLDIR)/libzip/usr/lib/libzip.so.5.5
	cd $(INSTALLDIR)/libzip/usr/lib/ && $(STRIP) libzip.so.5.5 && ln -sf libzip.so.5.5 libzip.so && ln -sf libzip.so.5.5 libzip.so.5

libzip-clean:
	@rm -rf libzip/build
	@rm -rf libzip/staged
	$(call unpatch_files,libzip)

$(PHP_TARGET)/stamp-h1:
	$(call patch_files,$(PHP_TARGET))
	cd $(PHP_TARGET) && touch configure.ac && autoconf && \
		PKG_CONFIG_LIBDIR="" \
		PROF_FLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -DZIP_DISABLE_DEPRECATED -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -I$(TOP)/zlib/staged/usr/include -I$(TOP)/libxml2/staged/usr/include -I$(TOP)/libxml2/staged/usr/include/libxml2 -I$(TOP)/$(PCRE_TARGET)/staged/usr/include -I$(TOP)/libiconv/staged/usr/include \
			-I$(TOP)/libpng/staged/usr/include -I$(TOP)/libjpeg-turbo/staged/usr/include -I$(TOP)/sqlite/staged/usr/include -I$(TOP)/libzip/staged/usr/include \
			$(if $(TCONFIG_BCMARM),-fno-builtin,) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -DZIP_DISABLE_DEPRECATED -I$(TOP)/zlib/staged/usr/include -I$(TOP)/libxml2/staged/usr/include -I$(TOP)/libxml2/staged/usr/include/libxml2 -I$(TOP)/libxml2/staged/usr/include/libxml2 -I$(TOP)/$(PCRE_TARGET)/staged/usr/include -I$(TOP)/libiconv/staged/usr/include \
			-I$(TOP)/libpng/staged/usr/include -I$(TOP)/libjpeg-turbo/staged/usr/include -I$(TOP)/sqlite/staged/usr/include -I$(TOP)/libzip/staged/usr/include \
			$(if $(TCONFIG_BCMARM),-fno-builtin,) -ffunction-sections -fdata-sections" \
		CXXFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CXXMODE) -DZIP_DISABLE_DEPRECATED $(if $(TCONFIG_BCMARM),-fno-builtin,) -ffunction-sections -fdata-sections" \
		LDFLAGS="-L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -L$(TOP)/sqlite/staged/usr/lib -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/libxml2/staged/usr/lib -L$(TOP)/libiconv/staged/usr/lib \
			-L$(TOP)/libpng/staged/usr/lib -L$(TOP)/libjpeg-turbo/staged/usr/lib -L$(TOP)/libzip/staged/usr/lib \
			-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		LIBS="-L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -L$(TOP)/sqlite/staged/usr/lib -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/libxml2/staged/usr/lib -L$(TOP)/libiconv/staged/usr/lib -L$(TOP)/libpng/staged/usr/lib \
			-L$(TOP)/libjpeg-turbo/staged/usr/lib -L$(TOP)/libzip/staged/usr/lib \
			-lz -lsqlite3 -ldl -lpthread -liconv -lxml2 -lstdc++ -ljpeg -lzip" \
		ac_cv_func_memcmp_working=yes \
		ac_cv_c_bigendian_php=no \
		php_cv_cc_rpath=no \
		ac_cv_php_xml2_config_path="$(TOP)/libxml2/staged/usr/bin/xml2-config" \
		ac_cv_u8t_decompose=yes \
		ac_cv_have_pcre2_jit=no \
		$(if $(TCONFIG_BCMARM),,ac_cv_header_atomic_h=no) \
		$(CONFIGURE) --prefix=/usr \
			--enable-cli \
			$(if $(TCONFIG_BCMARM),--enable-fpm --disable-cgi,--enable-cgi) \
			--enable-shared \
			--disable-static \
			--disable-rpath \
			--disable-debug \
			--disable-phpdbg \
			--with-config-file-path=/etc \
			--with-config-file-scan-dir=/jffs/etc \
			--with-zlib \
			--with-zlib-dir="$(TOP)/zlib/staged/usr" \
			--with-jpeg-dir="$(TOP)/libjpeg-turbo/staged/usr" \
			--with-jpeg="$(TOP)/libjpeg-turbo/staged/usr" \
			--with-png-dir="$(TOP)/libpng/staged/usr" \
			--with-iconv="$(TOP)/libiconv/staged/usr" \
			--with-iconv-dir="$(TOP)/libiconv/staged/usr" \
			--with-libxml-dir="$(TOP)/libxml2/staged/usr" \
			--with-pdo-sqlite="$(TOP)/sqlite/staged/usr" \
			--with-sqlite3="$(TOP)/sqlite/staged/usr" \
			--with-libzip="$(TOP)/libzip/staged/usr" \
			--with-pcre-dir="$(TOP)/$(PCRE_TARGET)/staged/usr" \
			--with-pcre-regex="$(TOP)/$(PCRE_TARGET)/staged/usr" \
			--with-external-pcre \
			--enable-ctype \
			--enable-fileinfo \
			--enable-dom \
			--enable-exif \
			--enable-hash \
			--enable-json \
			--enable-mbstring \
			--with-mysqli \
			--with-mysql-sock="/var/run/mysqld.sock" \
			--with-pdo-mysql \
			--enable-pdo \
			--enable-session \
			--enable-simplexml \
			--enable-xml \
			--enable-xmlreader \
			--enable-xmlwriter \
			--with-zip \
			--enable-zip \
			--with-gd \
			--enable-gd \
			--without-valgrind \
			--without-pear \
			--disable-short-tags \
			--disable-phar \
			--disable-calendar \
			--without-gettext \
			--disable-ftp \
			--without-gmp \
			--without-ldap \
			--without-openssl \
			--without-imap \
			--without-snmp \
			--without-curl \
			--disable-pcntl \
			--without-pdo-pgsql \
			--without-pgsql \
			--disable-soap \
			--disable-tokenizer \
			--with-kerberos=no \
			--without-freetype-dir \
			--without-xpm-dir \
			--disable-intl \
			--disable-gd-jis-conv \
			--disable-mbregex \
			--disable-opcache \
			--enable-sockets \
			LIBXML_CFLAGS="$(TOP)/libxml2/staged/usr/include/libxml2" \
			LIBXML_LIBS="$(TOP)/libxml2/staged/usr/lib -lxml2" \
			SQLITE_CFLAGS="$(TOP)/sqlite/staged/usr/include" \
			SQLITE_LIBS="$(TOP)/sqlite/staged/usr/lib -lsqlite3" \
			ZLIB_CFLAGS="$(TOP)/zlib/staged/usr/include" \
			ZLIB_LIBS="$(TOP)/zlib/staged/usr/lib -lz" \
			LIBZIP_CFLAGS="$(TOP)/libzip/staged/usr/include" \
			LIBZIP_LIBS="$(TOP)/libzip/staged/usr/lib -lzip" \
			JPEG_CFLAGS="$(TOP)/libjpeg-turbo/staged/usr/include" \
			JPEG_LIBS="$(TOP)/libjpeg-turbo/staged/usr/lib -ljpeg" \
			PNG_CFLAGS="(TOP)/libpng/staged/usr/include" \
			PNG_LIBS="$(TOP)/libpng/staged/usr/lib -lpng16" \
			PCRE2_CFLAGS="$(TOP)/$(PCRE_TARGET)/staged/usr/include" \
			PCRE2_LIBS="$(TOP)/$(PCRE_TARGET)/staged/usr/lib -lpcre2-8"
	@touch $@

$(PHP_TARGET): $(PHP_TARGET)/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

$(PHP_TARGET)-install:
	install -d $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin
	install -D $(PHP_TARGET)/sapi/cli/php $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cli && chmod 0755 $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cli
	$(STRIP) $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cli
ifeq ($(TCONFIG_BCMARM),y)
	install -D $(PHP_TARGET)/sapi/fpm/php-fpm $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-fpm && chmod 0755 $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-fpm
	$(STRIP) $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-fpm
else
	install -D $(PHP_TARGET)/sapi/cgi/php-cgi $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cgi && chmod 0755 $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cgi
	$(STRIP) $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin/php-cgi
	cd $(INSTALLDIR)/$(PHP_TARGET)/usr/sbin && ln -sf php-cgi php-fcgi
endif

$(PHP_TARGET)-clean:
	-@$(MAKE) -C $(PHP_TARGET) clean
	@rm -f $(PHP_TARGET)/stamp-h1
	$(call unpatch_files,$(PHP_TARGET))

libatomic_ops/stamp-h1:
	cd libatomic_ops && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
		$(CONFIGURE) --prefix=/usr --enable-static --disable-shared
	@touch $@

libatomic_ops: libatomic_ops/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

libatomic_ops-install:
	@true

libatomic_ops-clean:
	-@$(MAKE) -C libatomic_ops clean
	@rm -f libatomic_ops/stamp-h1

nginx/stamp-h1:
	$(call patch_files,nginx)
	cd nginx && \
		./configure --crossbuild=Linux::$(ARCH) \
			--with-cc="$(CC)" \
			--with-cc-opt="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -D_GNU_SOURCE -I$(TOP)/$(PCRE_TARGET)/staged/usr/include -I$(TOP)/zlib $(if $(TCONFIG_WOLFSSL),,-I$(TOP)/$(OPENSSLDIR)/include)" \
			--with-ld-opt="-L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -L$(TOP)/zlib/staged/usr/lib $(if $(TCONFIG_WOLFSSL),-L$(TOP)/wolfssl/staged/usr/lib,-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib)" \
			--prefix=/usr \
			--sbin-path=/usr/sbin \
			--conf-path=/etc/nginx/nginx.conf \
			--error-log-path=/var/log/nginx/error.log \
			--http-log-path=/var/log/nginx/access.log \
			--pid-path=/var/run/nginx.pid \
			--lock-path=/var/run/nginx.lock \
			--http-client-body-temp-path=/var/lib/nginx/client \
			--http-fastcgi-temp-path=/var/lib/nginx/fastcgi \
			--http-uwsgi-temp-path=/var/lib/nginx/uwsgi \
			--http-scgi-temp-path=/var/lib/nginx/scgi \
			--http-proxy-temp-path=/var/lib/nginx/proxy \
			--with-threads \
			--with-http_flv_module \
			--with-http_ssl_module \
			--with-http_gzip_static_module \
			--with-http_v2_module \
			--with-http_realip_module \
			--without-http_upstream_zone_module \
			$(if $(TCONFIG_WOLFSSL),--with-wolfssl=$(TOP)/wolfssl/staged/usr,) \
			$(if $(TCONFIG_BCMARM),,--with-libatomic=$(TOP)/libatomic_ops) \
			$(if $(TCONFIG_IPV6),--with-ipv6,)
	@touch $@

nginx: nginx/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -D_GNU_SOURCE -I$(TOP)/zlib -I$(TOP)/$(PCRE_TARGET)/staged/usr/include \
		$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl,-I$(TOP)/$(OPENSSLDIR)/include)" $(PARALLEL_BUILD)

nginx-install:
	install -D nginx/objs/nginx $(INSTALLDIR)/nginx/usr/sbin/nginx && chmod 0755 $(INSTALLDIR)/nginx/usr/sbin/nginx
	$(STRIP) $(INSTALLDIR)/nginx/usr/sbin/nginx

nginx-clean:
	-@$(MAKE) -C nginx clean
	@rm -f nginx/stamp-h1
	$(call unpatch_files,nginx)

libncurses/stamp-h1:
	$(call patch_files,libncurses)
	cd libncurses && \
		CFLAGS="-Os -Wall -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-Os $(EXTRACFLAGS) -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		LIBS="-lstdc++" \
		$(CONFIGURE) --prefix=/usr --with-shared --with-normal --disable-debug --without-ada --without-manpages --without-progs --without-profile --disable-big-core \
			--without-tests --without-cxx --without-cxx-bindings --with-build-cppflags=-D_GNU_SOURCE --disable-rpath --disable-rpath-hack
	@touch $@

libncurses: libncurses/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libncurses/staged install

libncurses-install:
	install -d $(INSTALLDIR)/libncurses/usr/lib
	install -d $(INSTALLDIR)/libncurses/usr/share/terminfo
	install -d $(INSTALLDIR)/libncurses/usr/share/terminfo/l
	install -d $(INSTALLDIR)/libncurses/usr/share/terminfo/v
	install -d $(INSTALLDIR)/libncurses/usr/share/terminfo/x
	install -D -m 0644 libncurses/staged/usr/share/terminfo/l/linux $(INSTALLDIR)/libncurses/usr/share/terminfo/l
	install -D -m 0644 libncurses/staged/usr/share/terminfo/v/vt100 $(INSTALLDIR)/libncurses/usr/share/terminfo/v
	install -D -m 0644 libncurses/staged/usr/share/terminfo/v/vt220 $(INSTALLDIR)/libncurses/usr/share/terminfo/v
	install -D -m 0644 libncurses/staged/usr/share/terminfo/x/xterm $(INSTALLDIR)/libncurses/usr/share/terminfo/x
	install -D -m 0644 libncurses/staged/usr/share/terminfo/x/xterm-256color $(INSTALLDIR)/libncurses/usr/share/terminfo/x
	cd $(INSTALLDIR)/libncurses/usr/lib && ln -sf ../share/terminfo terminfo
	install libncurses/lib/libncurses.so.6.4 $(INSTALLDIR)/libncurses/usr/lib/libncurses.so.6
	$(STRIP) $(INSTALLDIR)/libncurses/usr/lib/libncurses.so.6
	cd $(INSTALLDIR)/libncurses/usr/lib/ && ln -sf libncurses.so.6 libncurses.so

libncurses-clean:
	-@$(MAKE) -C libncurses clean
	@rm -f libncurses/stamp-h1 libncurses/Makefile
	@rm -rf libncurses/staged
	$(call unpatch_files,libncurses)

mysql/target/Makefile:
	@mkdir -p mysql/host
	cd mysql/host && \
		cmake -DWITH_SSL=system \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_C_FLAGS="" \
			-DCMAKE_CXX_FLAGS="" \
			-DCMAKE_C_COMPILER=$(CC:$(CROSS_COMPILE)%=%) \
			-DCMAKE_CXX_COMPILER=$(CXX:$(CROSS_COMPILE)%=%) \
			-DWITH_ZLIB=system \
			..
	$(MAKE) -C mysql/host comp_sql comp_err gen_lex_hash $(PARALLEL_BUILD)

	@cp -vf mysql/host/extra/comp_err mysql/host/comp_err
	@cp -vf mysql/host/sql/gen_lex_hash mysql/host/gen_lex_hash
	@cp -vf mysql/host/scripts/comp_sql mysql/host/comp_sql
	@cp -vf mysql/host/scripts/comp_sql mysql/scripts/comp_sql

	$(call patch_files,mysql)
	@mkdir -p mysql/target
	cd mysql/target && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr -DINSTALL_MYSQLSHAREDIR=share/mysql \
			-DCMAKE_BUILD_TYPE=$(if $(TCONFIG_BCMARM),Release,MinSizeRel) -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -fno-delete-null-pointer-checks -funit-at-a-time -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib/staged/usr/include -I$(TOP)/$(OPENSSLDIR)/staged/usr/include -I$(TOP)/libncurses/staged/usr/include" \
			-DCMAKE_CXX_FLAGS="-Wall $(EXTRA_CXXFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -fPIC" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -fPIC -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -L$(TOP)/libncurses/staged/usr/lib -lz -lssl -lcrypto -lncurses" \
			-DDISABLE_SHARED=FALSE \
			-DHAVE_LIBPTHREAD=TRUE \
			-DWITH_DEBUG=FALSE \
			-DSTACK_DIRECTION=TRUE \
			-DWITH_UNIT_TESTS=FALSE \
			-DWITH_EMBEDDED_SERVER=TRUE \
			-DWITH_EXAMPLE_STORAGE_ENGINE=FALSE \
			-DWITH_PARTITION_STORAGE_ENGINE=FALSE \
			-DWITH_FEDERATED_STORAGE_ENGINE=FALSE \
			-DWITH_NDBCLUSTER_STORAGE_ENGINE=FALSE \
			-DCURSES_INCLUDE_PATH="$(TOP)/libncurses/staged/usr/include" \
			-DCURSES_LIBRARY="$(TOP)/libncurses/staged/usr/lib/libncurses.so" \
			-DWITH_ZLIB=system \
			-DZLIB_INCLUDE_DIR="$(TOP)/zlib/staged/usr/include" \
			-DZLIB_LIBRARY="$(TOP)/zlib/staged/usr/lib/libz.so" \
			-DWITH_SSL="$(TOP)/$(OPENSSLDIR)/staged/usr" \
			-DOPENSSL_INCLUDE_DIR="$(TOP)/$(OPENSSLDIR)/staged/usr/include" \
			-DOPENSSL_LIBRARY="$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libssl.so" \
			-DCRYPTO_LIBRARY="$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libcrypto.so" \
			..

mysql: mysql/target/Makefile
	@$(SEP)
	@$(MAKE) -C mysql/target $(PARALLEL_BUILD)
	@$(MAKE) -C mysql/target DESTDIR=$(TOP)/mysql/staged install

mysql-install:
	install -d $(INSTALLDIR)/mysql/usr/bin
	install -d $(INSTALLDIR)/mysql/usr/lib
	install -d $(INSTALLDIR)/mysql/usr/lib/plugin
	install -d $(INSTALLDIR)/mysql/usr/libexec
	install -d $(INSTALLDIR)/mysql/usr/share
	install -d $(INSTALLDIR)/mysql/usr/share/mysql
	install -D -m 755 mysql/staged/usr/bin/my_print_defaults $(INSTALLDIR)/mysql/usr/bin
	$(STRIP) $(INSTALLDIR)/mysql/usr/bin/my_print_defaults
	install -D -m 755 mysql/staged/usr/bin/myisamchk $(INSTALLDIR)/mysql/usr/bin
	$(STRIP) $(INSTALLDIR)/mysql/usr/bin/myisamchk
	install -D -m 755 mysql/staged/usr/bin/mysql $(INSTALLDIR)/mysql/usr/bin
	$(STRIP) $(INSTALLDIR)/mysql/usr/bin/mysql
	install -D -m 755 mysql/staged/usr/scripts/mysql_install_db $(INSTALLDIR)/mysql/usr/bin
	install -D -m 755 mysql/staged/usr/bin/mysqladmin $(INSTALLDIR)/mysql/usr/bin
	$(STRIP) $(INSTALLDIR)/mysql/usr/bin/mysqladmin
	install -D -m 755 mysql/staged/usr/bin/mysqldump $(INSTALLDIR)/mysql/usr/bin
	$(STRIP) $(INSTALLDIR)/mysql/usr/bin/mysqldump
	install -D -m 755 mysql/staged/usr/bin/mysqld $(INSTALLDIR)/mysql/usr/libexec
	$(STRIP) $(INSTALLDIR)/mysql/usr/libexec/mysqld
	cd $(INSTALLDIR)/mysql/usr/bin && ln -sf ../libexec/mysqld mysqld
#	install -D -m 755 mysql/staged/usr/bin/mysqld_safe $(INSTALLDIR)/mysql/usr/bin
	install -D -m 755 mysql/staged/usr/lib/libmysqlclient.so.18.0.0 $(INSTALLDIR)/mysql/usr/lib
	$(STRIP) $(INSTALLDIR)/mysql/usr/lib/libmysqlclient.so.18.0.0
	-@cd $(INSTALLDIR)/mysql/usr/lib && \
		ln -sf libmysqlclient.so.18.0.0 libmysqlclient.so.18 && \
		ln -sf libmysqlclient.so.18.0.0 libmysqlclient.so
	install -D -m 755 mysql/staged/usr/lib/libmysqlclient_r.so.18.0.0 $(INSTALLDIR)/mysql/usr/lib
	$(STRIP) $(INSTALLDIR)/mysql/usr/lib/libmysqlclient_r.so.18.0.0
	-@cd $(INSTALLDIR)/mysql/usr/lib && \
		ln -sf libmysqlclient_r.so.18.0.0 libmysqlclient_r.so.18 && \
		ln -sf libmysqlclient_r.so.18.0.0 libmysqlclient_r.so
#	-@cd $(INSTALLDIR)/mysql/usr/lib/plugin && cp -arfpu $(TOP)/mysql/staged/usr/lib/plugin/* . && \
#	rm -f *.la *.a && \
#	$(STRIP) *.so.*
	-@cd $(INSTALLDIR)/mysql/usr/share/mysql && \
	cp -arfpu $(TOP)/mysql/staged/usr/share/mysql/english . && \
	cp -arfpu $(TOP)/mysql/staged/usr/share/mysql/fill_help_tables.sql . && \
	cp -arfpu $(TOP)/mysql/staged/usr/share/mysql/mysql_system_tables.sql . && \
	cp -arfpu $(TOP)/mysql/staged/usr/share/mysql/mysql_system_tables_data.sql .

mysql-clean:
	-@$(MAKE) -C mysql clean
	@rm -f mysql/scripts/comp_sql
	@rm -rf mysql/host mysql/target mysql/staged
	$(call unpatch_files,mysql)

lzo/stamp-h1:
	cd lzo && \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O3,$(CFLAG_OPTIMIZE)) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="$(if $(TCONFIG_BCMARM),-O3,$(CFLAG_OPTIMIZE)) -Wall -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=/usr --enable-shared --enable-static
	@touch $@

lzo: lzo/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/lzo/staged install

lzo-install:
ifeq ($(TCONFIG_TINC),y)
	install -D lzo/src/.libs/liblzo2.so.2.0.0 $(INSTALLDIR)/lzo/usr/lib/liblzo2.so.2.0.0
	$(STRIP) $(INSTALLDIR)/lzo/usr/lib/liblzo2.so.2.0.0
	cd $(INSTALLDIR)/lzo/usr/lib && \
		ln -sf liblzo2.so.2.0.0 liblzo2.so.2 && \
		ln -sf liblzo2.so.2.0.0 liblzo2.so
endif
	@true

lzo-clean:
	-@$(MAKE) -C lzo clean
	@rm -f lzo/stamp-h1
	@rm -rf lzo/staged

libcap-ng/stamp-h1:
	cd libcap-ng && ./autogen.sh && \
		CFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=/usr --enable-shared --enable-static=no --bindir=/usr/sbin --libdir=/usr/lib --without-python --without-python3 \
		ac_cv_prog_swig_found=no
	@touch $@

libcap-ng: libcap-ng/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

libcap-ng-install:
	install -D libcap-ng/src/.libs/libcap-ng.so.0.0.0 $(INSTALLDIR)/libcap-ng/usr/lib/libcap-ng.so.0.0.0
	$(STRIP) $(INSTALLDIR)/libcap-ng/usr/lib/libcap-ng.so.0.0.0
	cd $(INSTALLDIR)/libcap-ng/usr/lib && ln -sf libcap-ng.so.0.0.0 libcap-ng.so.0
ifneq ($(wildcard libcap-ng/src/.libs/libdrop_ambient.so.0.0.0),)
	install -D libcap-ng/src/.libs/libdrop_ambient.so.0.0.0 $(INSTALLDIR)/libcap-ng/usr/lib/libdrop_ambient.so.0.0.0
	$(STRIP) $(INSTALLDIR)/libcap-ng/usr/lib/libdrop_ambient.so.0.0.0
	cd $(INSTALLDIR)/libcap-ng/usr/lib && ln -sf libdrop_ambient.so.0.0.0 libdrop_ambient.so.0 && \
		ln -sf libdrop_ambient.so.0.0.0 libdrop_ambient.so
endif

libcap-ng-clean:
	$(MAKE) -C libcap-ng clean
	@rm -f libcap-ng/configure

openvpn-2.5/Makefile:
	$(call patch_files,openvpn-2.5)
	cd openvpn-2.5 && autoreconf -fsi && \
		OPENSSL_CFLAGS="-I$(TOP)/$(OPENSSLDIR)/staged/usr/include" \
		OPENSSL_LIBS="-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -lcrypto -lssl" \
		WOLFSSL_CFLAGS="-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl" \
		WOLFSSL_INCLUDEDIR="-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl" \
		WOLFSSL_LIBS="-L$(TOP)/wolfssl/staged/usr/lib -lwolfssl" \
		LZ4_CFLAGS="-I$(TOP)/lz4/staged/usr/include" \
		LZ4_LIBS="-L$(TOP)/lz4/staged/usr/lib -llz4" \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O2,$(CFLAG_OPTIMIZE)) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="$(if $(TCONFIG_WOLFSSL),-L$(TOP)/wolfssl/staged/usr/lib,-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib) -lpthread -ldl -Wl,--gc-sections" \
		CPPFLAGS="$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl,-I$(TOP)/$(OPENSSLDIR)/staged/usr/include)" \
		PLUGINDIR="/lib" IPROUTE="/usr/sbin/ip" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib $(if $(TCONFIG_WOLFSSL),--with-crypto-library=wolfssl,--with-crypto-library=openssl --with-openssl-engine=no) \
			$(if $(TOMATO_EXPERIMENTAL),,--disable-debug) --disable-plugin-auth-pam --disable-plugin-down-root --disable-pf --disable-unit-tests --disable-dependency-tracking \
			$(if $(or $(TCONFIG_BCMARM),$(TCONFIG_AIO)),--enable-management,--disable-management) \
			--enable-iproute2 ac_cv_lib_resolv_gethostbyname=no \
			--disable-lzo $(if $(TCONFIG_OPTIMIZE_SIZE_MORE),--disable-lz4,) $(if $(TCONFIG_BCMARM),,$(if $(TCONFIG_AIO),,--enable-small))
	@touch openvpn-2.5/.conf

openvpn-2.5: openvpn-2.5/Makefile
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

openvpn-2.5-install:
	install -D openvpn-2.5/src/openvpn/openvpn $(INSTALLDIR)/openvpn-2.5/usr/sbin/openvpn
	$(STRIP) -s $(INSTALLDIR)/openvpn-2.5/usr/sbin/openvpn
	chmod 0500 $(INSTALLDIR)/openvpn-2.5/usr/sbin/openvpn

openvpn-2.5-clean:
	-@$(MAKE) -C openvpn-2.5 clean
	@rm -f openvpn-2.5/Makefile
	$(call unpatch_files,openvpn-2.5)

openvpn/Makefile:
	$(call patch_files,openvpn)
	cd openvpn && autoreconf -fsi && \
		OPENSSL_CFLAGS="-I$(TOP)/$(OPENSSLDIR)/staged/usr/include" \
		OPENSSL_LIBS="-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -lcrypto -lssl" \
		WOLFSSL_CFLAGS="-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl" \
		WOLFSSL_INCLUDEDIR="-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl" \
		WOLFSSL_LIBS="-L$(TOP)/wolfssl/staged/usr/lib -lwolfssl" \
		LIBCAPNG_CFLAGS="-I$(TOP)/libcap-ng/src" \
		LIBCAPNG_LIBS="-L$(TOP)/libcap-ng/src/.libs -lcap-ng" \
		LZ4_CFLAGS="-I$(TOP)/lz4/staged/usr/include" \
		LZ4_LIBS="-L$(TOP)/lz4/staged/usr/lib -llz4" \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O2,$(CFLAG_OPTIMIZE)) -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(if $(TCONFIG_BCMARM),-I$(TOP)/libcap-ng/src,)" \
		LDFLAGS="$(if $(TCONFIG_WOLFSSL),-L$(TOP)/wolfssl/staged/usr/lib,-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib) -lpthread -ldl -Wl,--gc-sections" \
		CPPFLAGS="$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl,-I$(TOP)/$(OPENSSLDIR)/staged/usr/include)" \
		PLUGINDIR="/lib" IPROUTE="/usr/sbin/ip" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib $(if $(TCONFIG_WOLFSSL),--with-crypto-library=wolfssl,--with-crypto-library=openssl --with-openssl-engine=no) \
			--disable-selinux --disable-systemd --disable-pkcs11 --disable-dco \
			$(if $(TOMATO_EXPERIMENTAL),,--disable-debug) --disable-plugin-auth-pam --disable-plugin-down-root --disable-unit-tests --disable-dependency-tracking \
			$(if $(or $(TCONFIG_BCMARM),$(TCONFIG_AIO)),--enable-management,--disable-management) \
			--enable-iproute2 ac_cv_lib_resolv_gethostbyname=no \
			--disable-lzo $(if $(TCONFIG_OPTIMIZE_SIZE_MORE),--disable-lz4,) $(if $(TCONFIG_BCMARM),,$(if $(TCONFIG_AIO),,--enable-small))
	@touch openvpn/.conf

openvpn: openvpn/Makefile
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

openvpn-install:
	install -D openvpn/src/openvpn/.libs/openvpn $(INSTALLDIR)/openvpn/usr/sbin/openvpn
	$(STRIP) -s $(INSTALLDIR)/openvpn/usr/sbin/openvpn
	chmod 0500 $(INSTALLDIR)/openvpn/usr/sbin/openvpn

openvpn-clean:
	-@$(MAKE) -C openvpn clean
	@rm -f openvpn/Makefile
	$(call unpatch_files,openvpn)

nano/stamp-h1:
	cd nano && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -I$(TOP)/libncurses/staged/usr/include -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -I$(TOP)/libncurses/staged/usr/include -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libncurses/staged/usr/lib -fPIC" \
		NCURSES_LIBS="-lncurses" \
		ac_cv_lib_ncursesw_get_wch=no ac_cv_func_strcasecmp=no ac_cv_func_strncasecmp=no \
		$(CONFIGURE) --prefix=/usr --enable-tiny --disable-color --disable-utf8 --disable-nls --without-libiconv-prefix
	@touch $@

nano: nano/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

nano-install:
	install -d $(INSTALLDIR)/nano/usr/sbin
	install -D nano/src/nano $(INSTALLDIR)/nano/usr/sbin/nano
	$(STRIP) -s $(INSTALLDIR)/nano/usr/sbin/nano

nano-clean:
	-@$(MAKE) -C nano clean
	@rm -f nano/stamp-h1 nano/Makefile nano/src/Makefile

libcurl/stamp-h1:
	$(call patch_files,libcurl)
	cd libcurl && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACXXFLAGS) $(GCC_CMODE) -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -ffunction-sections -fdata-sections -fPIC \
			$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl,-I$(TOP)/$(OPENSSLDIR)/staged/usr/include)" \
		LDFLAGS="-Wl,--gc-sections -fPIC $(if $(TCONFIG_WOLFSSL),-Wl$(comma)-rpath$(comma)$(TOP)/wolfssl/staged/usr/lib,-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib)" \
		LIBS="-lpthread" \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			--enable-shared --enable-static --enable-cookies --enable-file --enable-ftp --enable-http --enable-tftp $(if $(TCONFIG_IPV6),--enable-ipv6,--disable-ipv6) \
			--with-zlib=$(TOP)/zlib/staged/usr $(if $(TCONFIG_WOLFSSL),--with-wolfssl=$(TOP)/wolfssl/staged/usr,--with-openssl=$(TOP)/$(OPENSSLDIR)/staged/usr $(if $(TCONFIG_STUBBY),--with-ca-fallback)) \
			--disable-dict --disable-debug --disable-gopher --disable-pthreads --disable-threaded-resolver --disable-progress-meter --disable-dateparse --disable-dnsshuffle \
			--disable-ldap --disable-manual --disable-telnet --disable-verbose --disable-doh --disable-mime --disable-netrc --disable-socketpair --disable-headers-api \
			--without-gnutls --without-libidn2 --without-libpsl $(if $(TCONFIG_BCMARM),,--disable-tls-srp) --without-ngtcp2 --without-nghttp3 --without-quiche --without-msh3 \
			--without-brotli --without-nghttp2 --without-zstd --without-librtmp --disable-versioned-symbols --enable-symbol-hiding --disable-proxy --disable-ntlm \
			--disable-ntlm-wb --disable-websockets --disable-alt-svc --disable-ares disable-libcurl-option --without-libgsasl --disable-docs --disable-dependency-tracking \
			--without-ca-path $(if $(TCONFIG_STUBBY),--with-ca-bundle=/etc/ssl/certs/ca-certificates.crt)
	@touch $@

libcurl: libcurl/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/libcurl/staged install
	@rm -f libcurl/staged/usr/lib/libcurl.la

libcurl-install:
	install -D libcurl/lib/.libs/libcurl.so.4.8.0 $(INSTALLDIR)/libcurl/usr/lib/libcurl.so.4.8.0
	$(STRIP) -s $(INSTALLDIR)/libcurl/usr/lib/libcurl.so.4.8.0
	cd $(INSTALLDIR)/libcurl/usr/lib/ && ln -sf libcurl.so.4.8.0 libcurl.so && ln -sf libcurl.so.4.8.0 libcurl.so.4
	install -D libcurl/src/.libs/curl $(INSTALLDIR)/libcurl/usr/sbin/curl
	$(STRIP) -s $(INSTALLDIR)/libcurl/usr/sbin/curl

libcurl-clean:
	-@$(MAKE) -C libcurl clean
	@rm -f libcurl/stamp-h1 libcurl/Makefile
	@rm -rf libcurl/staged
	$(call unpatch_files,libcurl)

libevent/stamp-h1:
	cd libevent && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC -I$(TOP)/zlib/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC -L$(TOP)/zlib/staged/usr/lib" \
		$(CONFIGURE) --prefix=/usr --disable-openssl --disable-doxygen-html \
			--disable-debug-mode --disable-samples --disable-dependency-tracking
	@touch $@

libevent: libevent/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libevent/staged install

libevent-install:
ifeq ($(or $(TCONFIG_BBT),$(TCONFIG_TOR)),y)
	install -D libevent/.libs/libevent-2.1.so.7.0.1 $(INSTALLDIR)/libevent/usr/lib/libevent-2.1.so.7
	$(STRIP) -s $(INSTALLDIR)/libevent/usr/lib/libevent-2.1.so.7
	cd $(INSTALLDIR)/libevent/usr/lib/ && ln -sf libevent-2.1.so.7 libevent.so
endif
	@true

libevent-clean:
	-@$(MAKE) -C libevent clean
	@rm -f libevent/stamp-h1 libevent/Makefile
	@rm -rf libevent/staged

libiconv/stamp-h1:
	cd libiconv && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS)  $(GCC_CMODE) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=/usr --disable-nls --disable-rpath --enable-static --enable-shared
	@touch $@

libiconv: libiconv/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libiconv/staged install

libiconv-clean:
	-@$(MAKE) -C libiconv clean
	@rm -f libiconv/stamp-h1 libiconv/Makefile
	@rm -rf libiconv/staged

libiconv-install:
	install -d $(INSTALLDIR)/libiconv/usr/lib
	install libiconv/lib/.libs/libiconv.so.2.7.0 $(INSTALLDIR)/libiconv/usr/lib/libiconv.so.2.7.0
	$(STRIP) -s $(INSTALLDIR)/libiconv/usr/lib/libiconv.so.2.7.0
	cd $(INSTALLDIR)/libiconv/usr/lib/ && \
		ln -sf libiconv.so.2.7.0 libiconv.so.2 && \
		ln -sf libiconv.so.2.7.0 libiconv.so

transmission/build/Makefile:
	@rm -rf transmission/build && mkdir -p transmission/build
	$(call patch_files,transmission)
	cd transmission/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=Release -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG $(GCC_CMODE) -ffunction-sections -fdata-sections \
				$(if $(TCONFIG_BCMARM),,--param large-function-growth=800 --param max-inline-insns-single=3600) \
				-I$(TOP)/zlib/staged/usr/include -I$(TOP)/libevent/staged/usr/include \
				$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl,-I$(TOP)/$(OPENSSLDIR)/staged/usr/include)" \
			-DCMAKE_CXX_FLAGS="-Wall $(EXTRA_CXXFLAGS) -DNDEBUG $(GCC_CXXMODE) -ffunction-sections -fdata-sections \
				$(if $(TCONFIG_BCMARM),,--param large-function-growth=800 --param max-inline-insns-single=3600) \
				-I$(TOP)/zlib/staged/usr/include -I$(TOP)/libevent/staged/usr/include \
				$(if $(TCONFIG_WOLFSSL),-I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl,-I$(TOP)/$(OPENSSLDIR)/staged/usr/include)" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/libevent/staged/usr/lib -lz -levent -lpthread -lstdc++ \
				$(if $(TCONFIG_WOLFSSL),-L$(TOP)/wolfssl/staged/usr/lib -lwolfssl,-L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -lssl -lcrypto)" \
			-DENABLE_LIGHTWEIGHT=ON -DENABLE_UTP=ON -DENABLE_UTILS=ON -DENABLE_NLS=OFF -DENABLE_CLI=OFF -DENABLE_GTK=OFF -DENABLE_QT=OFF -DENABLE_MAC=OFF -DENABLE_TESTS=OFF \
			-DUSE_SYSTEM_EVENT2=ON -DUSE_SYSTEM_DHT=OFF -DUSE_SYSTEM_UTP=OFF -DUSE_SYSTEM_B64=OFF -DUSE_SYSTEM_MINIUPNPC=OFF -DUSE_SYSTEM_NATPMP=OFF -DWITH_SYSTEMD=OFF -DINSTALL_DOC=OFF \
			-DWITH_CRYPTO=$(if $(TCONFIG_WOLFSSL),cyassl,openssl) \
			-DCURL_INCLUDE_DIR=$(TOP)/libcurl/include \
			-DCURL_LIBRARY=$(TOP)/libcurl/lib/.libs/libcurl.so \
			-DEVENT2_INCLUDE_DIR=$(TOP)/libevent/staged/usr/include \
			-DEVENT2_LIBRARY=$(TOP)/libevent/staged/usr/lib/libevent.so \
			$(if $(TCONFIG_WOLFSSL), \
				-DWOLFSSL_INCLUDE_DIR=$(TOP)/wolfssl/staged/usr/include \
				-DWOLFSSL_LIBRARY=$(TOP)/wolfssl/staged/usr/lib/libwolfssl.so \
				, \
				-DOPENSSL_INCLUDE_DIR=$(TOP)/$(OPENSSLDIR)/staged/usr/include \
				-DOPENSSL_CRYPTO_LIBRARY=$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libcrypto.so \
				-DOPENSSL_SSL_LIBRARY=$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libssl.so \
			) \
			-DZLIB_INCLUDE_DIR=$(TOP)/zlib/staged/usr/include \
			-DZLIB_LIBRARY=$(TOP)/zlib/staged/usr/lib/libz.so \
			-DICONV_INCLUDE_DIR="" \
			-DICONV_LIBRARY="" \
			..

transmission: transmission/build/Makefile
	@$(SEP)
	$(MAKE) -C transmission/build $(PARALLEL_BUILD)

transmission-install:
	$(MAKE) -C transmission/build DESTDIR=$(INSTALLDIR)/transmission install
	@rm -rf $(INSTALLDIR)/transmission/usr/bin/transmission-show
	@rm -rf $(INSTALLDIR)/transmission/usr/bin/transmission-edit
	@rm -rf $(INSTALLDIR)/transmission/usr/bin/transmission-create
	$(STRIP) -s $(INSTALLDIR)/transmission/usr/bin/transmission-daemon
ifneq ($(TCONFIG_TR_EXTRAS),y)
	@rm -rf $(INSTALLDIR)/transmission/usr/bin/transmission-remote
else
	$(STRIP) -s $(INSTALLDIR)/transmission/usr/bin/transmission-remote
endif

transmission-clean:
	-@$(MAKE) -C transmission clean
	@rm -rf transmission/build
	$(call unpatch_files,transmission)

libnfsidmap/stamp-h1:
	cd libnfsidmap && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS)" \
		ac_cv_func_malloc_0_nonnull=yes \
		ac_cv_func_realloc_0_nonnull=yes \
		$(CONFIGURE) --prefix=/usr --disable-shared --enable-static
	@touch $@

libnfsidmap: libnfsidmap/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libnfsidmap/staged install

libnfsidmap-install:
	@rm -f libnfsidmap/staged/usr/lib/libnfsidmap.la

libnfsidmap-clean:
	-@$(MAKE) -C libnfsidmap clean
	@rm -f libnfsidmap/stamp-h1
	@rm -rf libnfsidmap/staged

portmap:
	@$(SEP)
	$(MAKE) -C portmap CFLAGS="-Os -Wall $(EXTRACFLAGS)" NO_TCP_WRAPPER=y NO_PIE=y RPCUSER=nobody $(PARALLEL_BUILD)

portmap-install:
	install -D portmap/portmap $(INSTALLDIR)/portmap/usr/sbin/portmap
	$(STRIP) -s $(INSTALLDIR)/portmap/usr/sbin/portmap

portmap-clean:
	-@$(MAKE) -C portmap clean

e2fsprogs/stamp-h1:
	$(call patch_files,e2fsprogs)
	cd e2fsprogs && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections" \
		LIBS="-lgcc_s" \
		$(CONFIGURE) --prefix=/usr --sysconfdir=/etc --enable-elf-shlibs --with-crond-dir=no \
			--disable-tls --disable-nls --disable-jbd-debug --disable-blkid-debug --disable-testio-debug --disable-backtrace --disable-uuidd \
			--disable-debugfs --disable-imager --disable-resizer --disable-defrag --disable-fuse2fs --disable-e2initrd-helper --disable-rpath \
			$(if $(TCONFIG_BCMARM),ac_cv_lib_pthread_sem_init=no,) ac_cv_lib_dl_dlopen=no
	@touch $@

e2fsprogs: e2fsprogs/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

e2fsprogs-install:
	install -D e2fsprogs/e2fsck/e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/e2fsck
	install -D e2fsprogs/misc/mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mke2fs
	install -D e2fsprogs/misc/tune2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/tune2fs
ifeq ($(or $(TCONFIG_BCMARM),$(TCONFIG_AIO)),y)
	install -D e2fsprogs/misc/badblocks $(INSTALLDIR)/e2fsprogs/usr/sbin/badblocks
endif
	$(STRIP) -s $(INSTALLDIR)/e2fsprogs/usr/sbin/*
	install -D e2fsprogs/lib/libblkid.so.1.0 $(INSTALLDIR)/e2fsprogs/usr/lib/libblkid.so.1.0
	install -D e2fsprogs/lib/libcom_err.so.2.1 $(INSTALLDIR)/e2fsprogs/usr/lib/libcom_err.so.2.1
	install -D e2fsprogs/lib/libe2p.so.2.3 $(INSTALLDIR)/e2fsprogs/usr/lib/libe2p.so.2.3
	install -D e2fsprogs/lib/libext2fs.so.2.4 $(INSTALLDIR)/e2fsprogs/usr/lib/libext2fs.so.2.4
	install -D e2fsprogs/lib/libuuid.so.1.2 $(INSTALLDIR)/e2fsprogs/usr/lib/libuuid.so.1.2
	$(STRIP) -s $(INSTALLDIR)/e2fsprogs/usr/lib/*.so.*
	ln -sf libblkid.so.1.0 $(INSTALLDIR)/e2fsprogs/usr/lib/libblkid.so.1
	ln -sf libcom_err.so.2.1 $(INSTALLDIR)/e2fsprogs/usr/lib/libcom_err.so.2
	ln -sf libe2p.so.2.3 $(INSTALLDIR)/e2fsprogs/usr/lib/libe2p.so.2
	ln -sf libext2fs.so.2.4 $(INSTALLDIR)/e2fsprogs/usr/lib/libext2fs.so.2
	ln -sf libuuid.so.1.2 $(INSTALLDIR)/e2fsprogs/usr/lib/libuuid.so.1
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext2
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext3
ifeq ($(TCONFIG_BCMARM),y)
	ln -sf e2fsck $(INSTALLDIR)/e2fsprogs/usr/sbin/fsck.ext4
endif
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext2
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext3
ifeq ($(TCONFIG_BCMARM),y)
	ln -sf mke2fs $(INSTALLDIR)/e2fsprogs/usr/sbin/mkfs.ext4
endif
	install -D -m 0644 e2fsprogs/misc/mke2fs.conf $(INSTALLDIR)/rom/rom/etc/mke2fs.conf
	install -D -m 0644 e2fsprogs/e2fsck/e2fsck.conf $(INSTALLDIR)/rom/rom/etc/e2fsck.conf

e2fsprogs-clean:
	-@$(MAKE) -C e2fsprogs clean
	@rm -f e2fsprogs/stamp-h1 e2fsprogs/Makefile
	$(call unpatch_files,e2fsprogs)

nfs-utils/stamp-h1:
ifneq ($(TCONFIG_BCMARM),y)
	$(call patch_files,nfs-utils)
endif
	cd nfs-utils && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -I$(TOP)/libevent/staged/usr/include -I$(TOP)/libnfsidmap/staged/usr/include -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections -L$(TOP)/libevent/staged/usr/lib -L$(TOP)/libnfsidmap/staged/usr/lib" \
		knfsd_cv_bsd_signals=no \
		CC_FOR_BUILD=$(CC) \
		$(CONFIGURE) --disable-gss --without-tcp-wrappers --disable-nfsv4 --disable-ipv6 --disable-uuid --disable-mount \
			--disable-tirpc --disable-dependency-tracking $(if $(TCONFIG_BCMARM),--disable-nfsv41 --disable-nfsdcltrack,)
	@touch $@

nfs-utils: nfs-utils/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

nfs-utils-install:
	install -d $(INSTALLDIR)/nfs-utils/usr/sbin
	install -D nfs-utils/utils/nfsd/nfsd $(INSTALLDIR)/nfs-utils/usr/sbin/nfsd
	install -D nfs-utils/utils/showmount/showmount $(INSTALLDIR)/nfs-utils/usr/sbin/showmount
	install -D nfs-utils/utils/exportfs/exportfs $(INSTALLDIR)/nfs-utils/usr/sbin/exportfs
	install -D nfs-utils/utils/statd/statd $(INSTALLDIR)/nfs-utils/usr/sbin/statd
	install -D nfs-utils/utils/mountd/mountd $(INSTALLDIR)/nfs-utils/usr/sbin/mountd
	$(STRIP) -s $(INSTALLDIR)/nfs-utils/usr/sbin/*

nfs-utils-clean:
	-@$(MAKE) -C nfs-utils clean
	@rm -f nfs-utils/stamp-h1
ifneq ($(TCONFIG_BCMARM),y)
	$(call unpatch_files,nfs-utils)
endif

lz4:
	@$(SEP)
	cd lz4 && \
		CFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		CPPFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		CXXFLAGS="-O3 -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-Wl,--gc-sections -fPIC" \
		PREFIX=/usr \
		LIBDIR=/usr/lib \
		$(MAKE) LN_S="ln -sf" install DESTDIR=$(TOP)/lz4/staged

lz4-install:
ifeq ($(or $(INSTALL_LZ4),$(TCONFIG_TINC)),y)
	install -D lz4/lib/liblz4.so.1.10.0 $(INSTALLDIR)/lz4/usr/lib/liblz4.so.1.10.0
	$(STRIP) $(INSTALLDIR)/lz4/usr/lib/liblz4.so.1.10.0
	cd $(INSTALLDIR)/lz4/usr/lib && ln -sf liblz4.so.1.10.0 liblz4.so.1 && ln -sf liblz4.so.1.10.0 liblz4.so
endif
	@true

lz4-clean:
	-@$(MAKE) -C lz4 clean
	@rm -rf lz4/staged

tinc/build:
	cd tinc && \
		$(call meson_CrossOptions, meson-cross.txt) && \
		$(TOP)/meson/meson.py setup build \
		--cross-file meson-cross.txt \
		--prefix=/usr \
		-Dsysconfdir=/etc \
		-Dlocalstatedir=/var \
		-Dpkg_config_path="$(TOP)/zlib/staged/usr/lib/pkgconfig:$(TOP)/lzo/staged/usr/lib/pkgconfig:$(TOP)/lz4/staged/usr/lib/pkgconfig:$(TOP)/$(OPENSSLDIR)/staged/usr/lib/pkgconfig" \
		-Dbuildtype=release \
		-Dc_std=$(if $(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),gnu11,gnu99) \
		-Dwarning_level=1 \
		-Dc_args="$(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/zlib/staged/usr/include -I$(TOP)/lzo/staged/usr/include -I$(TOP)/lz4/staged/usr/include -I$(TOP)/$(OPENSSLDIR)/staged/usr/include" \
		-Dc_link_args="-Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/lzo/staged/usr/lib -L$(TOP)/lz4/staged/usr/lib -L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -lpthread" \
		-Dcrypto=openssl \
		-Dzlib=enabled \
		-Dlzo=enabled \
		-Dlz4=enabled \
		-Dhardening=true \
		-Db_staticpic=true \
		-Db_pie=true \
		-Dreadline=disabled \
		-Dcurses=disabled \
		-Dminiupnpc=disabled \
		-Dvde=disabled \
		-Dsystemd=disabled \
		-Ddocs=disabled \
		-Dtests=disabled

tinc: tinc/build
	@$(SEP)
	$(TOP)/meson/meson.py compile -C $@/build $(PARALLEL_BUILD)

tinc-install:
	install -D tinc/build/src/tinc $(INSTALLDIR)/tinc/usr/sbin/tinc
	install -D tinc/build/src/tincd $(INSTALLDIR)/tinc/usr/sbin/tincd
	$(STRIP) $(INSTALLDIR)/tinc/usr/sbin/tinc
	$(STRIP) $(INSTALLDIR)/tinc/usr/sbin/tincd

tinc-clean:
	@rm -rf tinc/build
	@rm -f tinc/meson-cross.txt

snmp/stamp-h1:
	$(call patch_files,snmp)
	cd snmp && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections $(OPTSIZE_FLAG)" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --with-persistent-directory=/tmp/snmp-persist --with-logfile=/var/log/snmpd.log \
		--disable-debugging --disable-manuals --disable-scripts --disable-applications --disable-privacy --disable-developer \
		--disable-ipv6 --disable-mibs --disable-mib-loading --disable-embedded-perl --with-perl-modules=no --without-opaque-special-types \
		--without-openssl --without-rsaref --without-kmem-usage --without-rpm \
		--with-out-transports=UDPIPv6,TCPIPv6,AAL5PVC,IPX,TCP,Unix \
		--with-out-mib-modules=snmpv3mibs,agent_mibs,agentx,notification,utilities,target \
		--with-mib-modules="\
			agent/extend,\
			host/hr_device,host/hr_disk,host/hr_filesys,host/hr_network,host/hr_partition,host/hr_print,host/hr_proc,host/hrSWRunTable,host/hr_storage,host/hr_system,\
			mibII/at,mibII/icmp,mibII/ifTable,mibII/ip,mibII/ipAddr,mibII/kernel_linux,mibII/snmp_mib,mibII/sysORTable,mibII/system_mib,mibII/tcp,mibII/udp,mibII/vacm_context,mibII/vacm_vars,mibII/var_route,\
			ucd-snmp/disk_hw,ucd-snmp/dlmod,ucd-snmp/extensible,ucd-snmp/loadave,ucd-snmp/logmatch,ucd-snmp/memory,ucd-snmp/pass,ucd-snmp/proc,ucd-snmp/proxy,ucd-snmp/vmstat,\
			util_funcs,if-mib/ifXTable,ip-mib/inetNetToMediaTable"\
		--with-default-snmp-version=2 --with-sys-contact=root --with-sys-location=Unknown \
		--with-endianness=little --enable-mini-agent --enable-shared=no --enable-static --with-gnu-ld \
		--enable-internal-md5 --enable-mfd-rewrites --with-defaults --with-copy-persistent-files=no
	@touch $@

snmp: snmp/stamp-h1
	@$(SEP)
	$(MAKE) -C snmp

snmp-install:
	install -D snmp/agent/snmpd $(INSTALLDIR)/snmp/usr/sbin/snmpd
	$(STRIP) $(INSTALLDIR)/snmp/usr/sbin/snmpd

snmp-clean:
	-@$(MAKE) -C snmp clean
	@rm -f snmp/stamp-h1
	$(call unpatch_files,snmp)

apcupsd/stamp-h1:
	$(call patch_files,apcupsd)
	cd apcupsd && touch autoconf/variables.mak && $(MAKE) configure && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CXXFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall -ffunction-sections -fdata-sections" \
		LDFLAGS="-L$(TOOLCHAIN)/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --bindir=/bin --sysconfdir=/etc/apcupsd --with-cgi-bin=/www/apcupsd \
			--enable-usb --enable-cgi --disable-lgd --enable-net \
			--disable-dumb --without-x --with-serial-dev= \
			$(if $(TCONFIG_AIO),--enable-pcnet --enable-snmp,--disable-pcnet --disable-snmp)
	@touch $@

apcupsd: apcupsd/stamp-h1
	@$(SEP)
	$(MAKE) -C apcupsd $(PARALLEL_BUILD)

apcupsd-install:
	$(MAKE) -C apcupsd DESTDIR=$(INSTALLDIR)/apcupsd install
	@rm -rf $(INSTALLDIR)/apcupsd/sbin/apctest
	@rm -rf $(INSTALLDIR)/apcupsd/www/apcupsd/ups*.cgi
	@mkdir -p $(TARGETDIR)/rom/etc/apcupsd
	@cp -f $(INSTALLDIR)/apcupsd/etc/apcupsd/* $(TARGETDIR)/rom/etc/apcupsd/
	@rm -rf $(INSTALLDIR)/apcupsd/etc
	$(STRIP) $(INSTALLDIR)/apcupsd/sbin/*
	$(STRIP) $(INSTALLDIR)/apcupsd/www/apcupsd/*
	@mkdir -p $(INSTALLDIR)/apcupsd/usr/bin
	@cd $(INSTALLDIR)/apcupsd/usr/bin && ln -sf ../../bin/hostname hostname

apcupsd-clean:
	-@$(MAKE) -C apcupsd clean
	@rm -f apcupsd/stamp-h1
	@rm -f apcupsd/config*
	$(call unpatch_files,apcupsd)

libsodium/stamp-h1:
	cd libsodium && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections -fPIC"  \
		$(CONFIGURE) --prefix=/usr --disable-ssp --enable-minimal --without-pthreads --disable-shared --enable-static --disable-dependency-tracking
	@touch $@

libsodium: libsodium/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

libsodium-install:
	@true

libsodium-clean:
	-@$(MAKE) -C libsodium clean
	@rm -f libsodium/stamp-h1

dnscrypt/stamp-h1:
	$(call patch_files,dnscrypt)
	cd dnscrypt && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -I$(TOP)/libsodium/src/libsodium/include -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -I$(TOP)/libsodium/src/libsodium/include -I$(TOP)/zlib/staged/usr/include -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections -L$(TOP)/libsodium/src/libsodium/.libs -L$(TOP)/zlib/staged/usr/lib" \
		$(CONFIGURE) --prefix=/usr --disable-ssp --disable-plugins --disable-dependency-tracking
	@touch $@

dnscrypt: dnscrypt/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

dnscrypt-install:
	install -D dnscrypt/src/proxy/dnscrypt-proxy $(INSTALLDIR)/dnscrypt/usr/sbin/dnscrypt-proxy
	install -D dnscrypt/src/hostip/hostip $(INSTALLDIR)/dnscrypt/usr/sbin/hostip
	$(STRIP) -s $(INSTALLDIR)/dnscrypt/usr/sbin/dnscrypt-proxy
	$(STRIP) -s $(INSTALLDIR)/dnscrypt/usr/sbin/hostip

dnscrypt-clean:
	-@$(MAKE) -C dnscrypt clean
	@rm -f dnscrypt/stamp-h1 dnscrypt/Makefile
	@rm -rf dnscrypt/src/proxy/.deps
	$(call unpatch_files,dnscrypt)

libyaml/stamp-h1:
	cd libyaml && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --sysconfdir=/etc --enable-static --disable-shared
	@touch $@

libyaml: libyaml/stamp-h1
	@$(SEP)
	$(MAKE) -C libyaml $(PARALLEL_BUILD)

libyaml-install:
	@true

libyaml-clean:
	-@$(MAKE) -C libyaml clean
	@rm -f libyaml/stamp-h1
	@rm -rf libyaml/src/.deps libyaml/src/tests/.deps

getdns/build/Makefile:
	@rm -rf getdns/build && mkdir -p getdns/build
	$(call patch_files,getdns)
	cd getdns/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG $(GCC_CMODE) -ffunction-sections -fdata-sections -I$(TOP)/$(OPENSSLDIR)/staged/usr/include -I$(TOP)/libyaml/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections $(if $(or $(TCONFIG_OPENSSL11),$(TCONFIG_OPENSSL30)),-lpthread,) -L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -L$(TOP)/libyaml/src/.libs -lssl -lcrypto -lyaml" \
			-DENABLE_STATIC=TRUE -DENABLE_SHARED=FALSE -DENABLE_GOST=FALSE \
			-DBUILD_GETDNS_QUERY=FALSE \
			-DBUILD_GETDNS_SERVER_MON=FALSE \
			-DBUILD_STUBBY=TRUE -DENABLE_STUB_ONLY=TRUE \
			-DBUILD_LIBEV=FALSE -DBUILD_LIBEVENT2=FALSE -DBUILD_LIBUV=FALSE \
			-DBUILD_TESTING=FALSE \
			-DOPENSSL_INCLUDE_DIR=$(TOP)/$(OPENSSLDIR)/staged/usr/include \
			-DOPENSSL_CRYPTO_LIBRARY=$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libcrypto.so \
			-DOPENSSL_SSL_LIBRARY=$(TOP)/$(OPENSSLDIR)/staged/usr/lib/libssl.so \
			-DLIBYAML_DIR=$(TOP)/libyaml \
			-DLIBYAML_INCLUDE_DIR=$(TOP)/libyaml/include \
			-DLIBYAML_LIBRARY=$(TOP)/libyaml/src/.libs/libyaml.a \
			-DCMAKE_DISABLE_FIND_PACKAGE_Libsystemd=TRUE \
			-DUSE_LIBIDN2=FALSE \
			-DFORCE_COMPAT_STRPTIME=TRUE \
			..

getdns: getdns/build/Makefile
	$(MAKE) -C getdns/build $(PARALLEL_BUILD)

getdns-install:
	install -d $(INSTALLDIR)/getdns/usr/sbin
	install -D getdns/build/stubby/stubby $(INSTALLDIR)/getdns/usr/sbin/stubby
	$(STRIP) -s $(INSTALLDIR)/getdns/usr/sbin/stubby

getdns-clean:
	@rm -rf getdns/build
	$(call unpatch_files,getdns)

tor/stamp-h1:
	cd tor && autoreconf -fsi && \
		CFLAGS="$(if $(TCONFIG_BCMARM),-O3,-Os) -Wall $(EXTRACFLAGS) -I$(TOP)/$(OPENSSLDIR)/staged/usr/include -ffunction-sections -fdata-sections $(if $(TCONFIG_KEYGEN),,-DOPENSSL_NO_ENGINE)" \
		CPPFLAGS="-I$(TOP)/$(OPENSSLDIR)/staged/usr" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --with-libevent-dir=$(TOP)/libevent/staged/usr \
			--with-openssl-dir=$(TOP)/$(OPENSSLDIR)/staged/usr --with-zlib-dir=$(TOP)/zlib/staged/usr \
			--disable-asciidoc --disable-tool-name-check --disable-unittests --disable-lzma \
			--disable-seccomp --disable-libscrypt --disable-zstd-advanced-apis \
			--disable-manpage --disable-html-manual --disable-dependency-tracking --disable-zstd --disable-systemd
	@touch $@

tor: tor/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

tor-install:
	install -D tor/src/app/tor $(INSTALLDIR)/tor/usr/sbin/tor
	$(STRIP) -s $(INSTALLDIR)/tor/usr/sbin/tor

tor-clean:
	-@$(MAKE) -C tor clean
	@rm -f tor/stamp-h1 tor/Makefile

udpxy/stamp-h1:
	$(call patch_files,udpxy)
	@touch $@

udpxy: udpxy/stamp-h1
	@$(SEP)
	NO_UDPXREC=yes \
	$(MAKE) -C udpxy lean CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections"

udpxy-install:
	install -d $(INSTALLDIR)/udpxy/usr/bin
	install -D udpxy/udpxy $(INSTALLDIR)/udpxy/bin/udpxy
	$(STRIP) -s $(INSTALLDIR)/udpxy/bin/udpxy

udpxy-clean:
	-@$(MAKE) -C udpxy clean
	@rm -f udpxy/stamp-h1
	$(call unpatch_files,udpxy)

mdadm:
	@$(SEP)
	$(MAKE) -C mdadm mdadm $(PARALLEL_BUILD)

mdadm-install:
	install -D mdadm/mdadm $(INSTALLDIR)/mdadm/usr/sbin/mdadm
	$(STRIP) -s $(INSTALLDIR)/mdadm/usr/sbin/mdadm

mdadm-clean:
	-@$(MAKE) -C mdadm clean

ipset/stamp-h1:
	$(call patch_files,ipset)
	cd ipset && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libmnl/staged/usr/lib -lmnl" \
		libmnl_CFLAGS="-I$(TOP)/libmnl/staged/usr/include" \
		libmnl_LIBS="-L$(TOP)/libmnl/staged/usr/lib -lmnl" \
		$(CONFIGURE) --prefix=/usr --with-kmod=no
	@touch $@

ipset: ipset/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

ipset-install:
	install -D ipset/src/ipset $(INSTALLDIR)/ipset/usr/sbin/ipset
	install -d $(INSTALLDIR)/ipset/usr/lib/
	install ipset/lib/.libs/libipset.so.11.1.0 $(INSTALLDIR)/ipset/usr/lib/libipset.so.11.1.0
	$(STRIP) $(INSTALLDIR)/ipset/usr/lib/libipset.so.11.1.0
	$(STRIP) $(INSTALLDIR)/ipset/usr/sbin/ipset
	cd $(INSTALLDIR)/ipset/usr/lib/ && \
		ln -sf libipset.so.11.1.0 libipset.so.11 && \
		ln -sf libipset.so.11.1.0 libipset.so

ipset-clean:
	-@$(MAKE) -C ipset clean
	@rm -f ipset/stamp-h1 ipset/Makefile
	$(call unpatch_files,ipset)

ipset-6.24:
	@$(SEP)
	$(call patch_files,ipset-6.24)
	$(MAKE) -C $@ binaries COPT_FLAGS="-Os -Wall $(EXTRACFLAGS) $(OPTSIZE_FLAG) -ffunction-sections -fdata-sections --param large-function-growth=800 --param max-inline-insns-single=3000"

ipset-6.24-install:
	install -D ipset-6.24/ipset $(INSTALLDIR)/ipset-6.24/usr/sbin/ipset
	install -d $(INSTALLDIR)/ipset-6.24/usr/lib/
	install ipset-6.24/*.so $(INSTALLDIR)/ipset-6.24/usr/lib/
	$(STRIP) $(INSTALLDIR)/ipset-6.24/usr/lib/*.so
	$(STRIP) $(INSTALLDIR)/ipset-6.24/usr/sbin/ipset

ipset-6.24-clean:
	-@$(MAKE) -C ipset-6.24 clean
	$(call unpatch_files,ipset-6.24)

libjson-c/build/Makefile:
	$(call patch_files,libjson-c)
	@rm -rf libjson-c/build && mkdir -p libjson-c/build
	cd libjson-c/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections" \
			-DBUILD_APPS=OFF \
			-DBUILD_SHARED_LIBS=OFF \
			-DDISABLE_EXTRA_LIBS=ON \
			$(if $(TCONFIG_BCMARM),,-DDISABLE_THREAD_LOCAL_STORAGE=TRUE -DDISABLE_WERROR=TRUE) \
			..

libjson-c: libjson-c/build/Makefile
	@$(SEP)
	@$(MAKE) -C libjson-c/build all $(PARALLEL_BUILD)
	@$(MAKE) -C libjson-c/build DESTDIR=$(TOP)/libjson-c/staged install

libjson-c-install:
	@true

libjson-c-clean:
	@rm -rf libjson-c/build
	@rm -rf libjson-c/staged
	$(call unpatch_files,libjson-c)

libubox/build/Makefile:
	$(call patch_files,libubox)
	@rm -rf libubox/build && mkdir -p libubox/build
	cd libubox/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) $(GCC_CMODE) -D_GNU_SOURCE $(if $(TCONFIG_BCMARM),-DTCONFIG_BCMARM,) -DNDEBUG \
				-ffunction-sections -fdata-sections -I$(TOP)/libjson-c/staged/usr/include" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections" \
			-DBUILD_LUA=OFF \
			-DBUILD_EXAMPLES=OFF \
			-Djson="$(TOP)/libjson-c/staged/usr/lib/libjson-c.a" \
			..

libubox: libubox/build/Makefile
	@$(SEP)
	@$(MAKE) -C libubox/build ubox-static blobmsg_json-static $(PARALLEL_BUILD)

libubox-install:
	@true
	
libubox-clean:
	@rm -rf libubox/build
	$(call unpatch_files,libubox)

uqmi/build/Makefile:
	$(call patch_files,uqmi)
	@rm -rf uqmi/build && mkdir -p uqmi/build
	cd uqmi/build && $(call CMAKE_CrossOptions, crosscompiled.cmake) && \
		cmake -DCMAKE_TOOLCHAIN_FILE=crosscompiled.cmake \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_SKIP_RPATH=TRUE \
			-DCMAKE_C_FLAGS="-Wall $(EXTRACFLAGS) -DNDEBUG -ffunction-sections -fdata-sections -I$(TOP)" \
			-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -lm -L$(TOP)/libubox/build" \
			-DBUILD_STATIC=TRUE \
			-DBUILD_UQMID=FALSE \
			-Djson_include_dir="$(TOP)/libjson-c/staged/usr/include" \
			-Djson_library="$(TOP)/libjson-c/staged/usr/lib/libjson-c.a" \
			-Dblobmsg_json_include_dir="$(TOP)" \
			-Dblobmsg_json_library="$(TOP)/libubox/build/libblobmsg_json.a" \
			-Dubox_include_dir="$(TOP)" \
			-Dubox_library="$(TOP)/libubox/build/libubox.a" \
			..

uqmi: uqmi/build/Makefile
	@$(SEP)
	@$(MAKE) -C uqmi/build $(PARALLEL_BUILD)

uqmi-install:
	install -D uqmi/build/uqmi/uqmi $(INSTALLDIR)/uqmi/usr/sbin/uqmi
	$(STRIP) $(INSTALLDIR)/uqmi/usr/sbin/uqmi

uqmi-clean:
	@rm -rf uqmi/build
	$(call unpatch_files,uqmi)

comgt:
	@$(SEP)
	$(call patch_files,comgt)
	@$(MAKE) -C $@ CFLAGS="-Os $(EXTRACFLAGS)" LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" comgt

comgt-install:
	install -D comgt/comgt $(INSTALLDIR)/comgt/usr/sbin/comgt
	cd $(INSTALLDIR)/comgt/usr/sbin/ && \
		ln -sf comgt gcom
	$(STRIP) -s $(INSTALLDIR)/comgt/usr/sbin/comgt
	@mkdir -p $(TARGETDIR)/rom/etc/gcom
	@cp -f comgt/gcom/* $(TARGETDIR)/rom/etc/gcom/

comgt-clean:
	-@$(MAKE) -C comgt clean
	$(call unpatch_files,comgt)

sd-idle:
	@$(SEP)
	$(MAKE) -C sd-idle CFLAGS="-Os -Wall $(EXTRACFLAGS)"

sd-idle-install:
	install -d $(INSTALLDIR)/sd-idle/usr/bin
	install -D -m 0755 sd-idle/sd-idle $(INSTALLDIR)/sd-idle/usr/bin/sd-idle
	$(STRIP) -s $(INSTALLDIR)/sd-idle/usr/bin/sd-idle

sd-idle-clean:
	-@$(MAKE) -C sd-idle clean

iperf/stamp-h1:
	$(call patch_files,iperf)
	cd iperf && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -D_GNU_SOURCE -fPIC" \
		LIBS="-lgcc_s" \
		ac_cv_func_clock_gettime="no" \
		ac_cv_func_daemon="no" \
		$(CONFIGURE) --prefix=/usr --disable-profiling --without-openssl --without-sctp
	@touch $@

iperf: iperf/stamp-h1
	@$(SEP)
	$(MAKE) -C iperf

iperf-install:
	install -D iperf/src/.libs/iperf3 $(INSTALLDIR)/iperf/usr/sbin/iperf
	install -d $(INSTALLDIR)/iperf/usr/lib/
	install iperf/src/.libs/libiperf.so.0.0.0 $(INSTALLDIR)/iperf/usr/lib/libiperf.so.0.0.0
	$(STRIP) $(INSTALLDIR)/iperf/usr/lib/libiperf.so.0.0.0
	$(STRIP) $(INSTALLDIR)/iperf/usr/sbin/iperf
	cd $(INSTALLDIR)/iperf/usr/lib/ && \
		ln -sf libiperf.so.0.0.0 libiperf.so.0 && \
		ln -sf libiperf.so.0.0.0 libiperf.so

iperf-clean:
	-@$(MAKE) -C iperf clean
	@rm -f iperf/stamp-h1 iperf/Makefile
	$(call unpatch_files,iperf)

libdaemon/stamp-h1:
	cd libdaemon && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr ac_cv_func_setpgrp_void=yes --disable-lynx --disable-examples --disable-dependency-tracking
	touch $@

libdaemon: libdaemon/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@
	@$(MAKE) -C $@ DESTDIR=$(TOP)/libdaemon/staged install

libdaemon-install:
	install -D libdaemon/libdaemon/.libs/libdaemon.so.0.5.0 $(INSTALLDIR)/libdaemon/usr/lib/libdaemon.so.0.5.0
	$(STRIP) $(INSTALLDIR)/libdaemon/usr/lib/*.so.*
	cd $(INSTALLDIR)/libdaemon/usr/lib && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so && \
		ln -sf libdaemon.so.0.5.0 libdaemon.so.0

libdaemon-clean:
	-@$(MAKE) -C libdaemon distclean
	@rm -rf libdaemon/staged
	@rm -f libdaemon/stamp-h1

expat/stamp-h1:
	cd expat && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CXXFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --without-docbook --without-examples --without-tests --disable-dependency-tracking
	touch $@

expat: expat/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/expat/staged install

expat-install:
	install -D expat/staged/usr/lib/libexpat.so.1.11.1 $(INSTALLDIR)/expat/usr/lib/libexpat.so.1.11.1
	$(STRIP) $(INSTALLDIR)/expat/usr/lib/libexpat.so.1.11.1
	cd $(INSTALLDIR)/expat/usr/lib && ln -sf libexpat.so.1.11.1 libexpat.so.1

expat-clean:
	-@$(MAKE) -C expat clean
	@rm -rf expat/staged
	@rm -f expat/stamp-h1

avahi/stamp-h1:
	$(call patch_files,avahi)
	cd avahi && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/expat/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/expat/staged/usr/lib -ldl -lpthread" \
		LIBDAEMON_CFLAGS="-I$(TOP)/libdaemon" \
		LIBDAEMON_LIBS="-L$(TOP)/libdaemon/libdaemon/.libs -ldaemon" \
		$(CONFIGURE) --prefix=/usr --sysconfdir=/etc localstatedir=/var --with-distro=none \
			--enable-introspection=no \
			--disable-nls --disable-glib --disable-libevent --disable-gobject \
			--disable-qt3 --disable-qt4 --disable-qt5 --disable-gtk --disable-gtk3 \
			--disable-dbus --disable-gdbm --disable-python --disable-python-dbus \
			--disable-mono --disable-monodoc --disable-autoipd \
			--disable-doxygen-doc --disable-manpages --disable-xmltoman \
			--with-xml=expat \
			--with-avahi-user="nobody" --with-avahi-group="nobody" \
			--disable-stack-protector \
			--disable-dependency-tracking \
			avahi_runtime_dir=/var/run servicedir=/etc/avahi/services
	@touch $@

avahi: avahi/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

avahi-install:
	install -D avahi/avahi-daemon/.libs/avahi-daemon $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	install -D avahi/avahi-common/.libs/libavahi-common.so.3.5.4 $(INSTALLDIR)/avahi/usr/lib/libavahi-common.so.3.5.4
	install -D avahi/avahi-core/.libs/libavahi-core.so.7.1.0 $(INSTALLDIR)/avahi/usr/lib/libavahi-core.so.7.1.0
	$(STRIP) $(INSTALLDIR)/avahi/usr/sbin/avahi-daemon
	$(STRIP) $(INSTALLDIR)/avahi/usr/lib/libavahi-common.so.3.5.4
	$(STRIP) $(INSTALLDIR)/avahi/usr/lib/libavahi-core.so.7.1.0
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-common.so.3.5.4 libavahi-common.so.3
	cd $(INSTALLDIR)/avahi/usr/lib && ln -sf libavahi-core.so.7.1.0 libavahi-core.so.7

avahi-clean:
	-@$(MAKE) -C avahi distclean
	@rm -f avahi/stamp-h1
	$(call unpatch_files,avahi)

wireguard-tools:
	$(call patch_files,wireguard-tools)
	WITH_BASHCOMPLETION=no WITH_EMBEDDED=yes WITH_WGQUICK=yes WITH_SYSTEMDUNITS=no PREFIX=$(TOP)/wireguard-tools/staged CFLAGS="-I uapi/linux $(EXTRACFLAGS)" \
	$(MAKE) -C wireguard-tools/src install
	@touch wireguard-tools/stamp-h1

wireguard-tools-install:
	install -D wireguard-tools/staged/bin/wg $(INSTALLDIR)/wireguard-tools/usr/sbin/wg
	install -D wireguard-tools/staged/bin/wg-quick $(INSTALLDIR)/wireguard-tools/usr/sbin/wg-quick
	$(STRIP) -s $(INSTALLDIR)/wireguard-tools/usr/sbin/wg
	chmod 0500 $(INSTALLDIR)/wireguard-tools/usr/sbin/wg
	chmod 0500 $(INSTALLDIR)/wireguard-tools/usr/sbin/wg-quick

wireguard-tools-clean:
	-@$(MAKE) -C wireguard-tools/src clean
	@rm -rf wireguard-tools/staged
	@rm -f wireguard-tools/stamp-h1
	$(call unpatch_files,wireguard-tools)

gettext-tiny/stamp-h1:
	$(MAKE) -C gettext-tiny \
		prefix=/usr \
		LIBINTL=NOOP \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libiconv/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libiconv/staged/usr/lib -liconv" \
		LIBS="-L$(TOP)/libiconv/staged/usr/lib -liconv"
	@touch $@

gettext-tiny: gettext-tiny/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ \
		prefix=/usr \
		LIBINTL=NOOP \
		DESTDIR=$(TOP)/$@/staged \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libiconv/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libiconv/staged/usr/lib -liconv" \
		LIBS="-L$(TOP)/libiconv/staged/usr/lib -liconv" \
		install

gettext-tiny-install:
	@true

gettext-tiny-clean:
	-@$(MAKE) -C gettext-tiny clean
	@rm -f gettext-tiny/stamp-h1
	@rm -rf gettext-tiny/staged

util-linux/stamp-h1:
	cd util-linux && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) \
			--prefix=/usr \
			--disable-nls \
			--disable-all-programs \
			--enable-libuuid \
			--enable-libblkid \
			--without-btrfs \
			--without-systemd \
			--without-python \
			--without-util \
			--without-selinux \
			--without-audit \
			--without-udev \
			--without-ncursesw \
			--without-ncurses \
			--without-slang \
			--without-tinfo \
			--without-readline \
			--without-utempter \
			--without-cap-ng \
			--without-libz \
			--without-libmagic \
			--without-user \
			--without-smack \
			--without-econf \
			--without-cryptsetup \
			--disable-hwclock \
			--disable-hwclock-cmos \
			--disable-shared
	@touch $@

util-linux: util-linux/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)
	@$(MAKE) -C $@ DESTDIR=$(TOP)/util-linux/staged install

util-linux-install:
	@true

util-linux-clean:
	-@$(MAKE) -C util-linux clean
	@rm -f util-linux/stamp-h1
	@rm -rf util-linux/staged

zfs/stamp-h1:
	$(call patch_files,zfs)
	cd zfs && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-I$(TOP)/zlib/staged/usr/include -I$(TOP)/util-linux/staged/usr/include -I$(TOP)/$(OPENSSLDIR)/staged/usr/include -I$(TOP)/gettext-tiny/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/util-linux/staged/usr/lib -L$(TOP)/$(OPENSSLDIR)/staged/usr/lib -L$(TOP)/gettext-tiny/staged/usr/lib" \
		$(CONFIGURE) \
			--prefix=/usr \
			--sysconfdir=/etc \
			--localstatedir=/var \
			--with-config=user \
			--disable-nls \
			--disable-pyzfs \
			--disable-shared
	@touch $@

zfs: zfs/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@ $(PARALLEL_BUILD)

zfs-install:
	install -D zfs/cmd/zfs/zfs $(INSTALLDIR)/zfs/usr/sbin/zfs
	install -D zfs/cmd/zpool/zpool $(INSTALLDIR)/zfs/usr/sbin/zpool
	$(STRIP) $(INSTALLDIR)/zfs/usr/sbin/zfs
	$(STRIP) $(INSTALLDIR)/zfs/usr/sbin/zpool
	cd $(INSTALLDIR)/zfs/usr/sbin/ && ln -sf zpool fsck.zpool

zfs-clean:
	-@$(MAKE) -C zfs clean
	@rm -f zfs/stamp-h1
	$(call unpatch_files,zfs)

libmnl/stamp-h1:
	cd libmnl && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --enable-static
	@touch $@

libmnl: libmnl/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libmnl/staged install
	@rm -f $(TOP)/libmnl/staged/usr/lib/libmnl.la

libmnl-install:
	install -d $(INSTALLDIR)/libmnl/usr/lib/
	install libmnl/src/.libs/libmnl.so.0.2.0 $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.2.0
	$(STRIP) $(INSTALLDIR)/libmnl/usr/lib/libmnl.so.0.2.0
	cd $(INSTALLDIR)/libmnl/usr/lib/ && \
		ln -sf libmnl.so.0.2.0 libmnl.so.0 && \
		ln -sf libmnl.so.0.2.0 libmnl.so

libmnl-clean:
	-@$(MAKE) -C libmnl clean
	@rm -f libmnl/stamp-h1
	@rm -rf libmnl/staged

libnetfilter_conntrack/stamp-h1:
	cd libnetfilter_conntrack && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libnfnetlink/include -I$(TOP)/libmnl/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libnfnetlink/src/.libs -L$(TOP)/libmnl/staged/usr/lib -lmnl" \
		PKG_CONFIG_PATH="$(TOP)/libnfnetlink:$(TOP)/libmnl/staged/usr/lib/pkgconfig" \
		$(CONFIGURE) --prefix=/usr
	@touch $@

libnetfilter_conntrack: libnetfilter_conntrack/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libnetfilter_conntrack/staged install
	@rm -f libnetfilter_conntrack/staged/usr/lib/libnetfilter_conntrack.la

libnetfilter_conntrack-install:
	install -d $(INSTALLDIR)/libnetfilter_conntrack/usr/lib/
	install libnetfilter_conntrack/src/.libs/libnetfilter_conntrack.so.3.8.0 \
	$(INSTALLDIR)/libnetfilter_conntrack/usr/lib/libnetfilter_conntrack.so.3.8.0
	$(STRIP) $(INSTALLDIR)/libnetfilter_conntrack/usr/lib/libnetfilter_conntrack.so.3.8.0
	cd $(INSTALLDIR)/libnetfilter_conntrack/usr/lib/ && \
		ln -sf libnetfilter_conntrack.so.3.8.0 libnetfilter_conntrack.so.3 && \
		ln -sf libnetfilter_conntrack.so.3.8.0 libnetfilter_conntrack.so

libnetfilter_conntrack-clean:
	-@$(MAKE) -C libnetfilter_conntrack clean
	@rm -f libnetfilter_conntrack/stamp-h1 libnetfilter_conntrack/Makefile
	@rm -rf libnetfilter_conntrack/staged

libnetfilter_log/stamp-h1:
	cd libnetfilter_log && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libnfnetlink/include -I$(TOP)/libmnl/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOOLCHAIN)/lib -L$(TOP)/libnfnetlink/src/.libs -L$(TOP)/libmnl/staged/usr/lib -lmnl" \
		PKG_CONFIG_PATH="$(TOP)/libnfnetlink:$(TOP)/libmnl/staged/usr/lib/pkgconfig" \
		$(CONFIGURE) --prefix=/usr --without-ipulog
	@touch $@

libnetfilter_log: libnetfilter_log/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libnetfilter_log/staged install

libnetfilter_log-install:
	install -d $(INSTALLDIR)/libnetfilter_log/usr/lib/
	install libnetfilter_log/src/.libs/libnetfilter_log.so.1.2.0 \
	$(INSTALLDIR)/libnetfilter_log/usr/lib/libnetfilter_log.so.1.2.0
	$(STRIP) $(INSTALLDIR)/libnetfilter_log/usr/lib/libnetfilter_log.so.1.2.0
	cd $(INSTALLDIR)/libnetfilter_log/usr/lib/ && \
		ln -sf libnetfilter_log.so.1.2.0 libnetfilter_log.so.1 && \
		ln -sf libnetfilter_log.so.1.2.0 libnetfilter_log.so

libnetfilter_log-clean:
	-@$(MAKE) -C libnetfilter_log clean
	@rm -f libnetfilter_log/stamp-h1 libnetfilter_log/Makefile
	@rm -rf libnetfilter_log/staged

libnetfilter_queue/stamp-h1:
	cd libnetfilter_queue && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libnfnetlink/include -I$(TOP)/libmnl/staged/usr/include -D_GNU_SOURCE=1" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libnfnetlink/src/.libs -L$(TOP)/libmnl/staged/usr/lib -lmnl" \
		PKG_CONFIG_PATH="$(TOP)/libnfnetlink:$(TOP)/libmnl/staged/usr/lib/pkgconfig" \
		$(CONFIGURE) --prefix=/usr
	@touch $@

libnetfilter_queue: libnetfilter_queue/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/libnetfilter_queue/staged install

libnetfilter_queue-install:
	install -d $(INSTALLDIR)/libnetfilter_queue/usr/lib/
	install libnetfilter_queue/src/.libs/libnetfilter_queue.so.1.5.0 \
	$(INSTALLDIR)/libnetfilter_queue/usr/lib/libnetfilter_queue.so.1.5.0
	$(STRIP) $(INSTALLDIR)/libnetfilter_queue/usr/lib/libnetfilter_queue.so.1.5.0
	cd $(INSTALLDIR)/libnetfilter_queue/usr/lib/ && \
		ln -sf libnetfilter_queue.so.1.5.0 libnetfilter_queue.so.1 && \
		ln -sf libnetfilter_queue.so.1.5.0 libnetfilter_queue.so

libnetfilter_queue-clean:
	-@$(MAKE) -C libnetfilter_queue clean
	@rm -f libnetfilter_queue/stamp-h1 libnetfilter_queue/Makefile
	@rm -rf libnetfilter_queue/staged

conntrack-tools/stamp-h1:
	cd conntrack-tools && autoreconf -fsi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libnfnetlink/include -I$(TOP)/libmnl/staged/usr/include \
			-I$(TOP)/libnetfilter_conntrack/staged/usr/include -I$(TOP)/libnetfilter_log/staged/usr/include -I$(TOP)/libnetfilter_queue/staged/usr/include" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libnfnetlink/src/.libs -L$(TOP)/libmnl/staged/usr/lib -lmnl \
			-L$(TOP)/libnetfilter_conntrack/staged/usr/lib -L$(TOP)/libnetfilter_log/staged/usr/lib -L$(TOP)/libnetfilter_queue/staged/usr/lib" \
		PKG_CONFIG_PATH="$(TOP)/libnfnetlink:$(TOP)/libmnl/staged/usr/lib/pkgconfig:$(TOP)/libnetfilter_conntrack/staged/usr/lib/pkgconfig:$(TOP)/libnetfilter_log/staged/usr/lib/pkgconfig:$(TOP)/libnetfilter_queue/staged/usr/lib/pkgconfig" \
		$(CONFIGURE) --prefix=/usr --disable-cthelper --disable-cttimeout
	@touch $@

conntrack-tools: conntrack-tools/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)
	$(MAKE) -C $@ DESTDIR=$(TOP)/conntrack-tools/staged install

conntrack-tools-install:
	install -d $(INSTALLDIR)/conntrack-tools/usr/sbin
	install conntrack-tools/src/conntrack $(INSTALLDIR)/conntrack-tools/usr/sbin/conntrack
	$(STRIP) -s $(INSTALLDIR)/conntrack-tools/usr/sbin/conntrack

conntrack-tools-clean:
	-@$(MAKE) -C conntrack-tools clean
	@rm -f conntrack-tools/stamp-h1 conntrack-tools/Makefile
	@rm -rf conntrack-tools/staged

eapd$(BCMEX)-clean:
	-@cd eapd$(BCMEX)/linux && make clean

ufsd-asus:
	@$(MAKE) -C $@ all

ufsd-asus-install:
	@$(MAKE) -C ufsd-asus INSTALLDIR=$(INSTALLDIR)/ufsd-asus install

diskdev_cmds-332.25:
	@$(SEP)
	$(call patch_files,diskdev_cmds-332.25)
	cd diskdev_cmds-332.25 && \
		make -f Makefile.lnx

diskdev_cmds-332.25-install:
	install -D diskdev_cmds-332.25/newfs_hfs.tproj/newfs_hfs $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/mkfs.hfsplus
	install -D diskdev_cmds-332.25/fsck_hfs.tproj/fsck_hfs $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/fsck.hfsplus
	$(STRIP) $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/mkfs.hfsplus
	$(STRIP) $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/fsck.hfsplus
	cd $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin && \
		rm -f mkfs.hfs && \
		rm -f fsck.hfs && \
		ln -s mkfs.hfsplus mkfs.hfs && \
		ln -s fsck.hfsplus fsck.hfs

diskdev_cmds-332.25-clean:
	-@cd diskdev_cmds-332.25 && make -f Makefile.lnx clean
	@rm -f $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/mkfs.hfs
	@rm -f $(INSTALLDIR)/diskdev_cmds-332.25/usr/sbin/fsck.hfs
	$(call unpatch_files,diskdev_cmds-332.25)

libffi/stamp-h1:
	cd libffi && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -fPIC" \
		LDFLAGS="-Wl,--gc-sections -fPIC" \
		$(CONFIGURE) --prefix=/usr --libdir=/usr/lib --disable-builddir --disable-docs --disable-multi-os-directory --disable-raw-api --disable-structs --disable-dependency-tracking
	@touch $@

libffi: libffi/stamp-h1
	@$(SEP)
	@$(MAKE) -C $@
	@$(MAKE) -C $@ DESTDIR=$(TOP)/libffi/staged install
	@rm -f libffi/staged/usr/lib/libffi.la

libffi-install:
ifeq ($(TCONFIG_IRQBALANCE),y)
	install -D $(TOP)/libffi/.libs/libffi.so.8.2.0 $(INSTALLDIR)/libffi/usr/lib/libffi.so.8.2.0
	$(STRIP) $(INSTALLDIR)/libffi/usr/lib/libffi.so.8.2.0
	cd $(INSTALLDIR)/libffi/usr/lib && ln -sf libffi.so.8.2.0 libffi.so.8 && ln -sf libffi.so.8.2.0 libffi.so
endif
	@true

libffi-clean:
	-@$(MAKE) -C libffi distclean
	@rm -rf libffi/staged
	@rm -f libffi/stamp-h1

glib2/build:
	$(call patch_files,glib2)
	cd glib2 && \
		$(call meson_CrossOptions, meson-cross.txt) && \
		$(TOP)/meson/meson.py setup build \
		--cross-file meson-cross.txt \
		--prefix=/usr \
		-Dpkg_config_path="$(TOP)/libiconv/staged/usr/lib/pkgconfig:$(TOP)/zlib/staged/usr/lib/pkgconfig:$(TOP)/libffi/staged/usr/lib/pkgconfig:$(TOP)/$(PCRE_TARGET)/staged/usr/lib/pkgconfig:$(TOP)/gettext-tiny/staged/usr/include" \
		-Dc_args="$(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libiconv/staged/usr/include -I$(TOP)/zlib/staged/usr/include \
			-I$(TOP)/libffi/staged/usr/include -I$(TOP)/$(PCRE_TARGET)/staged/usr/include -I$(TOP)/gettext-tiny/staged/usr/include" \
		-Dc_link_args="-Wl,--gc-sections -L$(TOP)/libiconv/staged/usr/lib -liconv -L$(TOP)/zlib/staged/usr/lib -lz -L$(TOP)/libffi/staged/usr/lib -lffi \
			-L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -lpcre2-8 -L$(TOP)/gettext-tiny/staged/usr/lib" \
		-Dcpp_args="$(EXTRACFLAGS) -ffunction-sections -fdata-sections -I$(TOP)/libiconv/staged/usr/include -I$(TOP)/zlib/staged/usr/include \
			-I$(TOP)/libffi/staged/usr/include -I$(TOP)/$(PCRE_TARGET)/staged/usr/include -I$(TOP)/gettext-tiny/staged/usr/include" \
		-Dcpp_link_args="-Wl,--gc-sections -L$(TOP)/libiconv/staged/usr/lib -liconv -L$(TOP)/zlib/staged/usr/lib -lz -L$(TOP)/libffi/staged/usr/lib -lffi \
			-L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -lpcre2-8 -L$(TOP)/gettext-tiny/staged/usr/lib" \
		-Dc_std=$(if $(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),gnu11,gnu99) \
		-Dcpp_std=$(if $(CONFIG_TOOLCHAIN53)$(CONFIG_TOOLCHAIN73),gnu++14,gnu++98) \
		-Dwarning_level=1 \
		-Doptimization=s \
		-Ddebug=false \
		-Dforce_posix_threads=true \
		-Db_staticpic=true \
		-Db_pie=true \
		-Dbsymbolic_functions=true \
		-Dglib_checks=true \
		-Dglib_debug=disabled \
		-Dlibmount=disabled \
		-Dselinux=disabled \
		-Dnls=disabled \
		-Dlibelf=disabled \
		-Dglib_assert=false \
		-Ddefault_library=shared \
		-Dsystemtap=false \
		-Dsysprof=disabled \
		-Doss_fuzz=disabled \
		-Dxattr=false \
		-Ddtrace=false \
		-Dgtk_doc=false \
		-Dman=false \
		-Dinstalled_tests=false \
		-Dtests=false

glib2: glib2/build
	@$(SEP)
	# workaround for new toolchain
	@cd $(SRCBASE) && mkdir -p usr/include
	$(TOP)/meson/meson.py compile -C $@/build $(PARALLEL_BUILD)
	$(TOP)/meson/meson.py install -C $@/build --destdir=$(TOP)/$@/staged

glib2-install:
	install -D glib2/staged/usr/lib/libglib-2.0.so.0.7400.7 $(INSTALLDIR)/glib2/usr/lib/libglib-2.0.so.0.7400.7
	$(STRIP) $(INSTALLDIR)/glib2/usr/lib/libglib-2.0.so.0.7400.7
	-@rm -rf $(INSTALLDIR)/glib2/usr/share
	cd $(INSTALLDIR)/glib2/usr/lib && ln -sf libglib-2.0.so.0.7400.7 libglib-2.0.so.0 && ln -sf libglib-2.0.so.0.7400.7 libglib-2.0.so

glib2-clean:
	@rm -rf glib2/build
	@rm -f glib2/meson-cross.txt
	$(call unpatch_files,glib2)

irqbalance/stamp-h1:
	$(call patch_files,irqbalance)
	cd irqbalance && ./autogen.sh && \
		CFLAGS="-Os -Wall $(EXTRACFLAGS) $(GCC_CMODE) -ffunction-sections -fdata-sections" \
		CPPFLAGS="-I$(TOP)/libiconv/staged/usr/include -I$(TOP)/libffi/staged/usr/include -I$(TOP)/zlib/staged/usr/include -I$(TOP)/$(PCRE_TARGET)/staged/usr/include" \
		LDFLAGS="-L$(TOP)/libiconv/staged/usr/lib -L$(TOP)/libffi/staged/usr/lib -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/$(PCRE_TARGET)/staged/usr/lib -Wl,--gc-sections" \
		LIBS="-lffi -liconv -lz -lpcre2-8" \
		GLIB2_CFLAGS="-I$(TOP)/glib2/staged/usr/lib/glib-2.0/include -I$(TOP)/glib2/staged/usr/include/glib-2.0" \
		GLIB2_LIBS="-L$(TOP)/glib2/staged/usr/lib -lglib-2.0" \
		$(CONFIGURE) --target=arm-linux --prefix=/usr --with-libcap_ng=no \
			--with-systemd=no --without-irqbalance-ui --disable-numa --disable-dependency-tracking
	@touch $@

irqbalance: irqbalance/stamp-h1
	@$(SEP)
	$(MAKE) -C $@

irqbalance-install: 
	install -D irqbalance/irqbalance $(INSTALLDIR)/irqbalance/usr/sbin/irqbalance
	$(STRIP) $(INSTALLDIR)/irqbalance/usr/sbin/irqbalance
	@rm -rf $(INSTALLDIR)/irqbalance/usr/share

irqbalance-clean: 
	-@$(MAKE) -C irqbalance clean
	@rm -f irqbalance/stamp-h1
	$(call unpatch_files,irqbalance)

haveged/stamp-h1:
	$(call patch_files,haveged)
	cd haveged && autoreconf -fsi && \
		CFLAGS="-Wall -Os $(EXTRACFLAGS) -ffunction-sections -fdata-sections" \
		LDFLAGS="-ffunction-sections -fdata-sections -Wl,--gc-sections" \
		$(CONFIGURE) --prefix=/usr --enable-static --disable-shared --enable-daemon --disable-tune --disable-olt
	@touch $@

haveged: haveged/stamp-h1
	@$(SEP)
	$(MAKE) -C $@ $(PARALLEL_BUILD)

haveged-install:
	install -D haveged/src/haveged $(INSTALLDIR)/haveged/usr/sbin/haveged
	$(STRIP) $(INSTALLDIR)/haveged/usr/sbin/haveged

haveged-clean:
	-@$(MAKE) -C haveged clean
	@rm -f haveged/stamp-h1
	$(call unpatch_files,haveged)

samba3/source3/Makefile:
	$(call patch_files,samba3)
	cd samba3/source3 && \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	ac_cv_header_libunwind_h=no \
	ac_cv_lib_resolv_dn_expand=no \
	ac_cv_lib_resolv__dn_expand=no \
	ac_cv_lib_resolv___dn_expand=no \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_linux_getgrouplist_ok=no \
	samba_cv_HAVE_BROKEN_FCNTL64_LOCKS=no \
	samba_cv_HAVE_BROKEN_GETGROUPS=no \
	$(if $(TCONFIG_BCMARM),samba_cv_HAVE_SENDFILE=yes,samba_cv_HAVE_SENDFILE=no) \
	$(if $(TCONFIG_IPV6),,libreplace_cv_HAVE_IPV6=no libreplace_cv_HAVE_IPV6_V6ONLY=no) \
	samba_cv_HAVE_BROKEN_LINUX_SENDFILE=yes \
	samba_cv_HAVE_BROKEN_READDIR_NAME=no \
	samba_cv_HAVE_DEV64_T=no \
	samba_cv_HAVE_DEVICE_MAJOR_FN=yes \
	samba_cv_HAVE_DEVICE_MINOR_FN=yes \
	samba_cv_HAVE_EXPLICIT_LARGEFILE_SUPPORT=yes \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_HAVE_FTRUNCATE_EXTEND=yes \
	samba_cv_HAVE_INO64_T=yes \
	samba_cv_HAVE_KERNEL_CHANGE_NOTIFY=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_KERNEL_SHARE_MODES=yes \
	samba_cv_HAVE_MAKEDEV=yes \
	samba_cv_HAVE_NATIVE_ICONV=no \
	samba_cv_HAVE_OFF64_T=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_STRUCT_FLOCK64=yes \
	samba_cv_HAVE_TRUNCATED_SALT=no \
	samba_cv_HAVE_UNSIGNED_CHAR=no \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_HAVE_Werror=yes \
	samba_cv_REALPATH_TAKES_NULL=yes \
	samba_cv_SIZEOF_DEV_T=yes \
	samba_cv_SIZEOF_INO_T=yes \
	samba_cv_SIZEOF_OFF_T=yes \
	samba_cv_SIZEOF_TIME_T=no \
	samba_cv_have_longlong=yes \
	samba_cv_have_setresuid=yes \
	samba_cv_have_setresgid=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_USE_SETREUID=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	LINUX_LFS_SUPPORT=yes \
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG="" \
	CFLAGS="$(EXTRACFLAGS) $(CFLAG_OPTIMIZE) $(if $(TCONFIG_BCMARM),-DBCMARM,) -DMAX_DEBUG_LEVEL="-1" -D__location__=\\\"\\\" -ffunction-sections -fdata-sections \
		$(if $(or $(TCONFIG_BCMARM),$(TCONFIG_NGINX)),-I$(TOP)/libiconv/staged/usr/include,) -I$(TOP)/zlib/staged/usr/include" \
	CPPFLAGS="-DNDEBUG -DSHMEM_SIZE=524288 -Dfcntl=fcntl64" \
	LDFLAGS="$(if $(or $(TCONFIG_BCMARM),$(TCONFIG_NGINX)),-L$(TOP)/libiconv/staged/usr/lib,) -L$(TOP)/zlib/staged/usr/lib -Wl,--gc-sections $(if $(TCONFIG_BCMARM),-lgcc_s,) -ldl -lpthread" \
	$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libdir=/etc \
		--localstatedir=/var \
		--enable-largefile \
		--disable-avahi \
		--disable-cups \
		--disable-debug \
		--disable-developer \
		--disable-dmalloc \
		--disable-external-libtalloc \
		--disable-external-libtdb \
		--disable-external-libtevent \
		--disable-fam \
		--disable-iprint \
		--disable-krb5developer \
		--disable-pie \
		--disable-relro \
		--disable-static \
		--disable-swat \
		--disable-shared-libs \
		--with-configdir=/etc/samba \
		--with-rootsbindir=/usr/sbin \
		--with-piddir=/var/run/samba \
		--with-privatedir=/etc/samba \
		--with-lockdir=/var/lock \
		--with-included-popt=no \
		--with-krb5=no \
		--with-shared-modules=MODULES \
		--with-included-iniparser \
		--with-logfilebase=/var/log \
		--with-nmbdsocketdir=/var/nmbd \
		$(if $(TCONFIG_BCMARM),--with-sendfile-support,) \
		$(if $(TCONFIG_BCMARM),--with-codepagedir=/usr/share/samba/codepages,) \
		$(if $(or $(TCONFIG_BCMARM),$(TCONFIG_NGINX)),--with-libiconv=$(TOP)/libiconv/staged/usr,) \
		--without-acl-support \
		--without-ads \
		--without-cluster-support \
		--without-dnsupdate \
		--without-krb5 \
		--without-ldap \
		--without-libaddns \
		--without-libtdb \
		--without-libnetapi \
		--without-libsmbclient \
		--without-libsmbsharemodes \
		--without-libtalloc \
		--without-libtevent \
		--without-pam \
		--without-quotas \
		--without-sys-quotas \
		--without-utmp \
		--without-winbind
	@mkdir -p samba3/source3/bin

samba3: samba3/source3/Makefile
	@$(SEP)
	$(MAKE) -C samba3/source3 all $(PARALLEL_BUILD)

samba3-install:
	@install -D samba3/source3/bin/samba_multicall $(INSTALLDIR)/samba3/usr/bin/samba_multicall
	@install -d $(INSTALLDIR)/samba3/usr/sbin/
	@cd $(INSTALLDIR)/samba3/usr/sbin && ln -sf ../bin/samba_multicall smbd && ln -sf ../bin/samba_multicall nmbd
	@cd $(INSTALLDIR)/samba3/usr/bin && ln -sf samba_multicall smbpasswd
ifeq ($(TCONFIG_BCMARM),y)
	@install -D samba3/codepages/lowcase.dat $(INSTALLDIR)/samba3/usr/share/samba/codepages/lowcase.dat
	@install -D samba3/codepages/upcase.dat $(INSTALLDIR)/samba3/usr/share/samba/codepages/upcase.dat
endif
	$(STRIP) -s $(INSTALLDIR)/samba3/usr/bin/samba_multicall

samba3-clean:
	-$(MAKE) -C samba3/source3 distclean
	@rm -f samba3/source3/auth/*.o
	@rm -f samba3/source3/Makefile
	$(call unpatch_files,samba3)


#
# Generic rules
#

%:
	@[ ! -d $* ] || ( $(SEP); $(MAKE) -C $* )

%-clean:
	@-[ ! -d $* ] || $(MAKE) -C $* clean

%-distclean:
	@-[ ! -d $* ] || $(MAKE) -C $* distclean

%-install:
	@[ ! -d $* ] || $(MAKE) -C $* install INSTALLDIR=$(INSTALLDIR)/$*

%-build:
	$(MAKE) $*-clean $*


$(obj-y) $(obj-n) $(obj-clean) $(obj-install): dummy

.PHONY: all clean distclean mrproper install package image
.PHONY: conf mconf oldconf kconf kmconf config menuconfig oldconfig
.PHONY: dummy libnet libpcap
