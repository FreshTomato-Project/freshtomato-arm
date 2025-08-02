include ../common.mak

CFLAGS  = -Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -Wsign-compare
CFLAGS += -I. -I$(SRCBASE)/include -idirafter$(LINUXDIR)/include -I$(TOP)/shared -I$(TOP)/nvram$(BCMEX)
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/shared -L$(TOP)/nvram$(BCMEX)
LIBS    = -lshared -lnvram -lcrypt $(EXTRALIBS)

ifeq ($(TCONFIG_DPSTA),y)
 CFLAGS += -I$(TOP)/dpsta
endif

ifeq ($(TCONFIG_BCMARM),y)
 CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include -DHAVE_TYPE_FLOAT
 CFLAGS  += -I$(SRCBASE)/common/include
 LDFLAGS += -L$(TOP)/libbcmcrypto -L$(TOP)/libbcm
 LIBS    += -lbcmcrypto -lbcm
endif

ifeq ($(TCONFIG_BCMSMP),y)
 CFLAGS += -DSMP
endif

ifeq ($(TOMATO_EXPERIMENTAL),1)
 CFLAGS += -DDEBUG_LOGMSG
endif

OBJS = rc.o init.o interface.o network.o wan.o services.o dhcp.o \
       firewall.o ppp.o telssh.o wnas.o listen.o redial.o \
       led.o qos.o forward.o misc.o mtd.o buttons.o restrict.o \
       gpio.o sched.o bwlimit.o arpbind.o tomatoanon.o mwan.o pbr.o \
       dnsmasq.o

ifeq ($(or $(TCONFIG_BCMARM),$(TCONFIG_BLINK)),y)
 OBJS += blink.o blink_br.o
endif

ifeq ($(TCONFIG_ROAM),y)
 OBJS += roamast.o
endif

ifeq ($(TCONFIG_BCMARM),y)
 OBJS += bcmutils.o
endif

ifeq ($(TCONFIG_BCMWL6),y)
 OBJS += arpreq.o
endif

ifeq ($(TCONFIG_FANCTRL),y)
 OBJS += phy_tempsense.o
endif

ifeq ($(TCONFIG_USB),y)
 OBJS += usb.o
endif

ifeq ($(TCONFIG_DDNS),y)
 OBJS += ddns.o
endif

ifeq ($(TCONFIG_CIFS),y)
 OBJS += cifs.o
endif

ifeq ($(TCONFIG_JFFS2),y)
 OBJS += jffs2.o
endif

ifeq ($(TCONFIG_SAMBASRV),y)
 OBJS += samba.o
endif

ifeq ($(TCONFIG_FTP),y)
 OBJS += ftpd.o
endif

ifeq ($(TCONFIG_OPENVPN),y)
 OBJS += openvpn.o
endif

ifeq ($(TCONFIG_WIREGUARD),y)
 OBJS += wireguard.o curve25519.o
endif

ifeq ($(TCONFIG_TINC),y)
 OBJS += tinc.o
endif

ifeq ($(TCONFIG_PPTPD),y)
 OBJS += pptpd.o pptp_client.o
endif

ifeq ($(TCONFIG_NOCAT),y)
 OBJS += nocat.o
endif

ifeq ($(TCONFIG_NGINX),y)
 OBJS += nginx.o mysql.o
endif

ifeq ($(TCONFIG_BT),y)
 OBJS += transmission.o
endif

ifeq ($(TCONFIG_NFS),y)
 OBJS += nfs.o
endif

ifeq ($(TCONFIG_SNMP),y)
 OBJS += snmp.o
endif

ifeq ($(TCONFIG_SDHC),y)
 OBJS += mmc.o
endif

ifeq ($(TCONFIG_UPS),y)
 OBJS += ups.o
endif

ifeq ($(TCONFIG_TOR),y)
 OBJS += tor.o
endif

ifeq ($(TCONFIG_BCMARM),y)
	vpath %.c $(SRCBASE)/shared
endif

all: rc

%.o: %.c
	@echo " [rc] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

rc: $(OBJS)
	@echo " [rc] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo " [rc] Installing as $(INSTALLDIR)/sbin/rc"
	@install -D rc $(INSTALLDIR)/sbin/rc
	@$(STRIP) $(INSTALLDIR)/sbin/rc
	@chmod 0500 $(INSTALLDIR)/sbin/rc

ifeq ($(TCONFIG_BCMARM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc preinit
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc init
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc console
	@cd $(INSTALLDIR)/sbin && ln -sf rc hotplug
	@cd $(INSTALLDIR)/sbin && ln -sf rc service
	@cd $(INSTALLDIR)/sbin && ln -sf rc buttons
ifeq ($(or $(TCONFIG_BCMARM),$(TCONFIG_BLINK)),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc blink
	@cd $(INSTALLDIR)/sbin && ln -sf rc blink_br
endif
ifeq ($(TCONFIG_FANCTRL),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc phy_tempsense
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc rcheck
	@cd $(INSTALLDIR)/sbin && ln -sf rc radio
	@cd $(INSTALLDIR)/sbin && ln -sf rc led
	@cd $(INSTALLDIR)/sbin && ln -sf rc reboot
	@cd $(INSTALLDIR)/sbin && ln -sf rc halt
	@cd $(INSTALLDIR)/sbin && ln -sf rc redial

	@cd $(INSTALLDIR)/sbin && ln -sf rc mwanroute

	@cd $(INSTALLDIR)/sbin && ln -sf rc gpio
	@cd $(INSTALLDIR)/sbin && ln -sf rc sched

	@cd $(INSTALLDIR)/sbin && ln -sf rc disconnected_pppoe
	@cd $(INSTALLDIR)/sbin && ln -sf rc listen
	@cd $(INSTALLDIR)/sbin && ln -sf rc ppp_event

	@cd $(INSTALLDIR)/sbin && ln -sf rc ntpd_synced

ifeq ($(TCONFIG_BCMARM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-write2
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-erase2
	@cd $(INSTALLDIR)/sbin && ln -sf rc nvram_erase
else
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-write
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-erase
endif
	@cd $(INSTALLDIR)/sbin && ln -sf rc mtd-unlock

	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc-event
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc-release
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc-renew
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcpc-event-lan

	@cd $(INSTALLDIR)/sbin && ln -sf rc wldist

ifeq ($(TCONFIG_IPV6),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc dhcp6c-state
endif

ifeq ($(TCONFIG_DDNS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc ddns-update
endif

ifeq ($(TCONFIG_CIFS),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc mount-cifs
endif

ifeq ($(TCONFIG_ROAM),y)
	@cd $(INSTALLDIR)/sbin && ln -sf rc roamast
endif

clean:
	@rm -f rc *.o
