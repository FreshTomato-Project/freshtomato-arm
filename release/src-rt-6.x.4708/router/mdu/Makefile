include ../common.mak

CFLAGS  = -Os -Wall $(EXTRACFLAGS) -ffunction-sections -fdata-sections -Wsign-compare
CFLAGS += -I$(SRCBASE)/include -I$(TOP)/shared -I$(TOP)/nvram$(BCMEX)
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/shared -L$(TOP)/nvram$(BCMEX)
LIBS    = -lshared -lnvram -lpthread $(EXTRALIBS)

ifneq ($(TCONFIG_HTTPS),)
 ifeq ($(TCONFIG_WOLFSSL),y)
  CFLAGS  += -DUSE_WOLFSSL -I$(TOP)/wolfssl/staged/usr/include -I$(TOP)/wolfssl/staged/usr/include/wolfssl -I$(TOP)/wolfssl
  LDFLAGS += -L$(TOP)/wolfssl/staged/usr/lib
  LIBS    += -lwolfssl
 else
  CFLAGS  += -I$(TOP)/$(OPENSSLDIR)/include
  LDFLAGS += -L$(TOP)/$(OPENSSLDIR)
  LIBS    += -lssl -lcrypto
 endif
endif

ifeq ($(or $(TCONFIG_BCMARM),$(TCONFIG_BBT),$(TCONFIG_NGINX)),y)
 CFLAGS  += -DUSE_LIBCURL -I$(TOP)/zlib/staged/usr/include -I$(TOP)/$(LIBCURL_TARGET)/staged/usr/include
 LDFLAGS += -L$(TOP)/zlib/staged/usr/lib -L$(TOP)/$(LIBCURL_TARGET)/staged/usr/lib
 LIBS    += -lz -lcurl
else
 CFLAGS  += -I$(TOP)/mssl
 LDFLAGS += -L$(TOP)/mssl
 LIBS    += -lmssl
endif

ifeq ($(TOMATO_EXPERIMENTAL),1)
 export CFLAGS += -DDEBUG_LOGMSG
endif

OBJS = mdu.o

all: mdu

%.o: %.c
	@echo " [mdu] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

mdu: $(OBJS)
	@echo " [mdu] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	@$(SIZECHECK)
	@$(CPTMP)

install: all
	@echo " [mdu] Installing as $(INSTALLDIR)/bin/mdu"
	@install -D mdu $(INSTALLDIR)/bin/mdu
	@$(STRIP) $(INSTALLDIR)/bin/mdu
	@chmod 0500 $(INSTALLDIR)/bin/mdu

clean:
	@rm -f mdu *.o
