include ../common.mak

CFLAGS  = -Wall $(EXTRACFLAGS) -fPIC -ffunction-sections -fdata-sections -Wsign-compare
CFLAGS += -I$(SRCBASE)/include -I. -I$(TOP)/shared
LDFLAGS = -fPIC -Wl,--gc-sections -L. -L$(TOP)/shared
LIBS    = -lshared -lnvram $(EXTRALIBS)

ifeq ($(TCONFIG_BCMARM),y)
 CFLAGS += -O2 -s
else
 CFLAGS += -Os
endif

ifeq ($(TCONFIG_BCMARM),y)
 CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include
endif

ifeq ($(TCONFIG_BCMARM),y)
vpath %.c $(SRCBASE)/shared/nvram
endif

all: libnvram.so nvram

%.o: %.c
	@echo " [nvram$(BCMEX)] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

libnvram.so: nvram_linux.o $(if $(TCONFIG_BCMARM),,nvram_convert.o)
	@echo " [nvram$(BCMEX)] LD $@"
	@$(LD) -shared -o $@ $^
	$(SIZECHECK)
	$(CPTMP)

nvram: libnvram.so $(if $(TCONFIG_BCMARM),main.o,nvram.o defaults.o)
	@echo " [nvram$(BCMEX)] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo " [nvram$(BCMEX)] Installing as $(INSTALLDIR)/usr/lib/libnvram.so"
	@install -D libnvram.so $(INSTALLDIR)/usr/lib/libnvram.so
	@$(STRIP) $(INSTALLDIR)/usr/lib/libnvram.so
	@echo " [nvram]$(BCMEX) Installing as $(INSTALLDIR)/bin/nvram"
	@install -D nvram $(INSTALLDIR)/bin/nvram
	@$(STRIP) $(INSTALLDIR)/bin/nvram
	@chmod 0555 $(INSTALLDIR)/bin/nvram

clean:
	@rm -f nvram *.o *.so
