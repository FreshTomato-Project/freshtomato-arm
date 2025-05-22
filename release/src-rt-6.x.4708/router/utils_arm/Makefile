#
# Miscellaneous utilities Makefile
#
# Copyright (C) 2009, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile 382640 2013-02-03 04:35:49Z $
#

include $(SRCBASE)/router/.config

WLU_OBJS := wlu.o wlu_cmd.o wlu_iov.o wlu_linux.o bcmutils.o bcmwifi_channels.o wlu_client_shared.o wlu_pipe.o wlu_pipe_linux.o miniopt.o bcm_app_utils.o wlu_rates_matrix.o wlc_ppr.o
ifneq ($(wildcard $(SRCBASE)/wl/exe/wlu_common.c),)
WLU_OBJS += wlu_common.o
endif

ifeq ($(RWL),1)
WL_OBJS := $(WLU_OBJS)
SERVER_OBJS := wlu_server_linux.o wlu_pipe_linux.o wlu_pipe.o wlu_server_shared.o shellproc_linux.o
APUT_OBJS := wfa_apcmdproc.o wfa_apcmdlib.o wfa_apcs.o wfa_apcmdresp.o wfa_apcmdvalid.o wfa_apcmdtbls.o wfa_apcmdcvt.o wfa_val2str.o wfa_apdiy.o wfa_sock.o wfa_aput_all.o

SOCKET_OBJS := $(SERVER_OBJS:%.o=socket/%.o)
WIFI_OBJS := $(SERVER_OBJS:%.o=wifi/%.o)
WL_OBJS := $(WL_OBJS:%.o=wl_obj/%.o) 
WFA_APUT_OBJS := $(APUT_OBJS:%.o=aput/%.o)

SERVER_SOCKET  := wl_server_socket
SERVER_WIFI  := wl_server_wifi
WFA_APUT_ALL := wfa_aput_all
WL_EXE  := wl
endif

UTILS := epi_ttcp
CFLAGS += -s -I$(SRCBASE)/include -I$(SRCBASE)/common/include
CFLAGS += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
ifeq ($(TCONFIG_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
CFLAGS += -DBCMINTERNAL -DBCMDBG

CFLAGS += -DLINUX -I$(SRCBASE)/wl/exe

# Use newer 11ac ratespec for wl command line
CFLAGS += -DD11AC_IOTYPES -DPPR_API

#ifdef WLCNT
CFLAGS += -DWLCNT
#endif

#ifeq ($(TCONFIG_5301X),y)
ifeq ($(TCONFIG_BCMARM),y)
CFLAGS += -DBCM5301X
endif

ifeq ($(CONFIG_TRAFFIC_MGMT),y)
CFLAGS += -DTRAFFIC_MGMT
endif

vpath %.c $(SRCBASE)/tools/misc $(WLAN_ComponentSrcDirs)

UTILS += robocfg

ifneq ($(wildcard $(SRCBASE)/et/linux/et.c),)
UTILS += et
vpath %.c $(SRCBASE)/et/linux
endif

ifeq ($(TCONFIG_BCMSMP),y) #for dual-core
ifneq ($(wildcard $(SRCBASE)/wl/arm-uclibc),)
CFLAGS += -I$(SRCBASE)/wl/arm-uclibc
UTILS += wl
vpath %.c $(SRCBASE)/wl/arm-uclibc $(SRCBASE)/shared
endif
else #for single-core
ifneq ($(wildcard $(SRCBASE)/wl/arm-uclibc/up),)
CFLAGS += -I$(SRCBASE)/wl/arm-uclibc/up
UTILS += wl
vpath %.c $(SRCBASE)/wl/arm-uclibc/up $(SRCBASE)/shared
endif
endif #($(TCONFIG_BCMSMP),y)


ifeq ($(RWL),1)
ifeq ($(BUILD_WFA_APUT),1)
ifneq ($(wildcard $(SRCBASE)/router/aput),)
CFLAGS += -I$(SRCBASE)/router/aput
UTILS += $(WFA_APUT_ALL)
vpath %.c $(SRCBASE)/router/aput
endif
endif

ifneq ($(wildcard $(SRCBASE)/wl/exe),)
UTILS += $(SERVER_SOCKET)
vpath %.c $(SRCBASE)/wl/exe $(SRCBASE)/shared
endif

ifneq ($(wildcard $(SRCBASE)/wl/exe),)
UTILS += $(SERVER_WIFI)
vpath %.c $(SRCBASE)/wl/exe $(SRCBASE)/shared
endif
endif

ifeq ($(TCONFIG_BCMSMP),y) #for dual-core util
ifneq ($(wildcard $(SRCBASE)/wl/arm-uclibc),)
all: robocfg et epi_ttcp
	-cp $(SRCBASE)/wl/arm-uclibc/wl wl
else
all: $(UTILS)
endif
else #for single-core util
ifneq ($(wildcard $(SRCBASE)/wl/arm-uclibc/up),)
all: robocfg et epi_ttcp
	-cp $(SRCBASE)/wl/arm-uclibc/up/wl wl
else
all: $(UTILS)
endif
endif #($(TCONFIG_BCMSMP),y)

wl: $(WLU_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

ifeq ($(RWL),1)
$(WFA_APUT_ALL): $(WFA_APUT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ 

aput/%.o: %.c
	@mkdir -p aput
	$(CC) -c $(CFLAGS) -o $@ $^
$(SERVER_SOCKET): $(SOCKET_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ 

socket/%.o: %.c
	@mkdir -p socket
	$(CC) -c $(CFLAGS) -DRWL_SOCKET -o $@ $^

$(SERVER_WIFI): $(WIFI_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

wifi/%.o: %.c
	@mkdir -p wifi
	$(CC) -c $(CFLAGS) -DRWL_WIFI -o $@ $^
endif

robocfg: robocfg.c etc53xx.h

clean:
	rm -f *.o *~ $(UTILS)
	rm -rf socket wifi

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install $(UTILS) $(INSTALLDIR)/usr/sbin
	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
