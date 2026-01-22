include ../common.mak

CFLAGS  = -Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -Wsign-compare
CFLAGS += -I$(SRCBASE)/include -I$(TOP)/shared -I$(TOP)/nvram$(BCMEX) $(if $(INSTALL_ZLIB),-I$(TOP)/zlib/staged/usr/include -DUSE_ZLIB,)
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/shared -L$(TOP)/nvram$(BCMEX) $(if $(INSTALL_ZLIB),-L$(TOP)/zlib/staged/usr/lib,)
LIBS    = -lshared -lnvram $(if $(INSTALL_ZLIB),-lz,) $(EXTRALIBS)

ifeq ($(TOMATO_EXPERIMENTAL),1)
 export CFLAGS += -DDEBUG_LOGMSG
endif

OBJS = rstats.o

all: rstats

%.o: %.c
	@echo " [rstats] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

rstats: $(OBJS)
	@echo " [rstats] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	$(SIZECHECK)
	$(CPTMP)

install: all
	@echo " [rstats] Installing as $(INSTALLDIR)/bin/rstats"
	@install -D rstats $(INSTALLDIR)/bin/rstats
	@$(STRIP) $(INSTALLDIR)/bin/rstats
	@chmod 0500 $(INSTALLDIR)/bin/rstats

clean:
	@rm -f rstats *.o
