HOSTCC ?= gcc

HOSTCFLAGS ?= -O2
HOSTCFLAGS += -Wall -DKBUILD_NO_NLS -DLOCALE -I.

HOSTNCURSES = $(shell sh lxdialog/check-lxdialog.sh -ccflags)
HOSTLOADLIBES_MCONF = $(shell sh lxdialog/check-lxdialog.sh -ldflags $(HOSTCC))

CONF_OBJS := \
	conf.o \
	zconf.tab.o

MCONF_OBJS := \
	mconf.o \
	zconf.tab.o \
	lxdialog/checklist.o \
	lxdialog/inputbox.o \
	lxdialog/menubox.o \
	lxdialog/textbox.o \
	lxdialog/util.o \
	lxdialog/yesno.o

.PHONY: all clean

all: conf mconf

conf: $(CONF_OBJS)
	$(HOSTCC) $(NATIVE_LDFLAGS) -o $@ $^

mconf: $(MCONF_OBJS)
	$(HOSTCC) $(NATIVE_LDFLAGS) -o $@ $^ $(HOSTLOADLIBES_MCONF)

conf.o: conf.c lkc.h lkc_proto.h expr.h
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

mconf.o: mconf.c lkc.h lkc_proto.h expr.h lxdialog/dialog.h
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTNCURSES) -c -o $@ $<

zconf.tab.o: zconf.tab.c zconf.lex.c \
	     lkc.h lkc_proto.h expr.h list.h \
	     confdata.c expr.c symbol.c menu.c util.c kconf_id.c
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ zconf.tab.c

lxdialog/%.o: lxdialog/%.c lxdialog/dialog.h
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTNCURSES) -c -o $@ $<

zconf.tab.c: zconf.tab.c_shipped
	cp $< $@

zconf.lex.c: zconf.lex.c_shipped
	cp $< $@

clean:
	rm -f \
		conf mconf \
		*.o lxdialog/*.o \
		zconf.tab.c zconf.lex.c
