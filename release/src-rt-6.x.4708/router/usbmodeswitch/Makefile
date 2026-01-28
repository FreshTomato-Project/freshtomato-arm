PROG        = usb_modeswitch
VERS        = 2.6.1
CC          ?= gcc
CFLAGS      += -Wall -Wno-deprecated-declarations
LIBS        = `pkg-config --libs --cflags libusb-1.0`
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
PREFIX      = $(DESTDIR)/usr
ETCDIR      = $(DESTDIR)/etc
SYSDIR      = $(ETCDIR)/systemd/system
UPSDIR      = $(ETCDIR)/init
UDEVDIR     = $(DESTDIR)/lib/udev
SBINDIR     = $(PREFIX)/sbin
MANDIR      = $(PREFIX)/share/man/man1

.PHONY: clean install install-common uninstall \
	dispatcher-script dispatcher-dynlink dispatcher-statlink \
	install-script install-dynlink install-statlink

all: all-with-script-dispatcher

all-with-script-dispatcher: $(PROG) dispatcher-script

all-with-dynlink-dispatcher: $(PROG) dispatcher-dynlink

all-with-statlink-dispatcher: $(PROG) dispatcher-statlink

$(PROG): $(OBJS) usb_modeswitch.h
	$(CC) -o $(PROG) $(OBJS) $(CFLAGS) $(LIBS) $(LDFLAGS)

dispatcher-script: usb_modeswitch_dispatcher.tcl
	DISPATCH=dispatcher-script
	cp -f usb_modeswitch_dispatcher.tcl usb_modeswitch_dispatcher

dispatcher-dynlink: dispatcher.c dispatcher.h
	DISPATCH=dispatcher-dynlink
	$(CC) dispatcher.c $(LDFLAGS) -Ljim -ljim -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

dispatcher-statlink: dispatcher.c dispatcher.h
	DISPATCH=dispatcher-statlink
	$(CC) dispatcher.c $(LDFLAGS) -l:libjim.a -ldl -Ijim -o usb_modeswitch_dispatcher $(CFLAGS)

dispatcher.h: usb_modeswitch_dispatcher.tcl
	./make_string.sh usb_modeswitch_dispatcher.tcl > $@

clean:
	$(RM) usb_modeswitch
	$(RM) usb_modeswitch_dispatcher
	$(RM) dispatcher.h

distclean: clean

# If the systemd folder is present, install the service for starting the dispatcher
# If not, use the dispatcher directly from the udev rule as in previous versions

install-common: $(PROG) $(DISPATCH)
	install -D --mode=755 usb_modeswitch $(SBINDIR)/usb_modeswitch
	install -D --mode=755 usb_modeswitch.sh $(UDEVDIR)/usb_modeswitch
	install -D --mode=644 usb_modeswitch.conf $(ETCDIR)/usb_modeswitch.conf
	install -D --mode=644 usb_modeswitch.1 $(MANDIR)/usb_modeswitch.1
	install -D --mode=644 usb_modeswitch_dispatcher.1 $(MANDIR)/usb_modeswitch_dispatcher.1
	install -D --mode=755 usb_modeswitch_dispatcher $(SBINDIR)/usb_modeswitch_dispatcher
	install -d $(DESTDIR)/var/lib/usb_modeswitch
	test -d $(UPSDIR) -a -e /sbin/initctl && install --mode=644 usb-modeswitch-upstart.conf $(UPSDIR) || test 1
	test -d $(SYSDIR) -a \( -e /usr/bin/systemctl -o -e /bin/systemctl \) && install --mode=644 usb_modeswitch@.service $(SYSDIR) || test 1

install: install-script

install-script: dispatcher-script install-common

install-dynlink: dispatcher-dynlink install-common

install-statlink: dispatcher-statlink install-common

uninstall:
	$(RM) $(SBINDIR)/usb_modeswitch
	$(RM) $(SBINDIR)/usb_modeswitch_dispatcher
	$(RM) $(UDEVDIR)/usb_modeswitch
	$(RM) $(ETCDIR)/usb_modeswitch.conf
	$(RM) $(MANDIR)/usb_modeswitch.1
	$(RM) -R $(DESTDIR)/var/lib/usb_modeswitch
	$(RM) $(SYSDIR)/usb_modeswitch@.service
