include ../common.mak

CFLAGS  = -Os -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections -Wsign-compare
CFLAGS += -I. -I$(SRCBASE)/include
LDFLAGS = -fPIC --gc-sections

ifeq ($(TCONFIG_BCMWL6),y)
 CFLAGS += -DTRAFFIC_MGMT
endif

ifeq (2_6_36,$(LINUX_VERSION))
 CFLAGS += -DAUTOFW_PORT_DEPRECATED
endif

ifeq ($(TCONFIG_BCMARM),y)
 CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/common/include
endif

ifeq ($(TCONFIG_BCMSMP),y)
 CFLAGS += -DSMP
endif

ifeq ($(TCONFIG_DHDAP),y)
 export CFLAGS += -D__CONFIG_DHDAP__
endif

ifeq ($(TCONFIG_BCMBSD),y)
 export CFLAGS += -DBCM_BSD
endif

ifeq ($(TOMATO_EXPERIMENTAL),1)
 export CFLAGS += -DDEBUG_LOGMSG
endif

OBJS = shutils.o wl.o wl_linux.o linux_timer.o defaults.o id.o misc.o \
       led.o version.o base64.o files.o strings.o process.o

# RT-N+
ifeq ($(or $(TCONFIG_BLINK),$(TCONFIG_BCMARM)),y)
 OBJS += nvparse.o
endif

ifeq ($(TCONFIG_USB),y)
 OBJS += bb_uuid.o \
         usb.o \
         $(TOP)/busybox/util-linux/volume_id/linux_swap.o \
         $(TOP)/busybox/util-linux/volume_id/ext.o \
         $(TOP)/busybox/util-linux/volume_id/util2.o \
         $(TOP)/busybox/util-linux/volume_id/fat.o \
         $(TOP)/busybox/util-linux/volume_id/ntfs.o
 ifeq ($(TCONFIG_BCMARM),y)
  OBJS += $(TOP)/busybox/util-linux/volume_id/exfat.o
 endif
 ifeq ($(TCONFIG_HFS),y)
  CFLAGS += -DHFS
  OBJS += $(TOP)/busybox/libbb/hash_md5_sha.o \
          $(TOP)/busybox/libbb/bb_bswap_64.o \
          $(TOP)/busybox/util-linux/volume_id/hfs.o
 endif
endif

ifeq ($(CONFIG_BCMWL5),y)
 OBJS += wlif_utils.o UdpLib.o
 ifeq ($(TCONFIG_BCMARM),y)
  OBJS += bcmutils.o bcmwifi_channels.o
 endif
endif

ifeq ($(TCONFIG_BCM7),y)
 OBJS += et_linux.o
endif

ifeq ($(TCONFIG_BCMARM),y)
vpath %.c $(SRCBASE)/shared/ $(SRCBASE)/shared/bcmwifi/src $(WLAN_ComponentSrcDirs)
endif

all: libshared.so

%.o: %.c
	@echo " [shared] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

libshared.so: $(OBJS)
	@echo " [shared] LD $@"
	@$(LD) -shared -o $@ $^ $(LDFLAGS)
	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo " [shared] Installing as $(INSTALLDIR)/usr/lib/libshared.so"
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libshared.so $(INSTALLDIR)/usr/lib
	@$(STRIP) $(INSTALLDIR)/usr/lib/libshared.so

clean:
	@rm -f *.o *.so
