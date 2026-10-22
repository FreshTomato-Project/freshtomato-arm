# Makefile.am -- cap-audit utility
#
# Copyright 2026 Red Hat Inc.
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# Authors:
#   Steve Grubb <sgrubb@redhat.com>
#

SUBDIRS = test
DIST_SUBDIRS = test

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src \
	-I$(top_srcdir)/utils \
	-I$(top_builddir)/utils/cap-audit ${LIBBPF_CFLAGS} ${LIBAUDIT_CFLAGS}
AM_CFLAGS = -W -Wall -Wshadow ${WFLAGS} -Wundef -D_GNU_SOURCE

if BUILD_CAP_AUDIT
bin_PROGRAMS = cap-audit
cap_audit_SOURCES = cap_audit.c \
	cap_audit_analyze.c \
	cap_audit_capset.c \
	cap_audit_event.c \
	cap_audit_json.c \
	cap_audit_outcome.c \
	cap_audit_path.c \
	cap_audit_path.h \
	cap_audit_service.c \
	classify_app.c \
	classify_app.h \
	cap_audit_util.c \
	cap_audit_yaml.c \
	cap_audit.h
nodist_cap_audit_SOURCES = cap_audit.skel.h
cap_audit_LDADD = ${top_builddir}/src/libcap-ng.la ${LIBBPF_LIBS} ${LIBAUDIT_LIBS}
endif

EXTRA_DIST = cap_audit.bpf.c cap-audit.8
CLEANFILES = cap_audit.bpf.o cap_audit.skel.h vmlinux.h

if BUILD_CAP_AUDIT
BUILT_SOURCES = cap_audit.bpf.o cap_audit.skel.h vmlinux.h
cap_audit.$(OBJEXT) cap_audit_analyze.$(OBJEXT) \
cap_audit_capset.$(OBJEXT) cap_audit_event.$(OBJEXT) \
cap_audit_json.$(OBJEXT) \
cap_audit_outcome.$(OBJEXT) cap_audit_service.$(OBJEXT) \
cap_audit_util.$(OBJEXT) \
cap_audit_yaml.$(OBJEXT): cap_audit.skel.h
endif

BPF_ARCH = @BPF_ARCH@
BPFTOOL = @BPFTOOL@
CLANG = @CLANG@
LLVM_STRIP = @LLVM_STRIP@
VMLINUX_H_PATH = @VMLINUX_H_PATH@

# Keep -g to generate the BTF and CO-RE metadata needed at load time.
BPF_CFLAGS = -g -O2 -target bpf -D__TARGET_ARCH_${BPF_ARCH} ${AM_CPPFLAGS}

# Do not reuse a partial BPF object or skeleton after a failed build step.
.DELETE_ON_ERROR:

cap_audit.skel.h: cap_audit.bpf.o
	$(AM_V_GEN)$(BPFTOOL) gen skeleton $< > $@

# The skeleton embeds the whole object, including vmlinux.h's large DWARF
# type descriptions. Strip only debug information before embedding it;
# --strip-debug preserves BTF, CO-RE relocations, and loader symbols.
cap_audit.bpf.o: cap_audit.bpf.c vmlinux.h
	$(AM_V_CC)$(CLANG) $(BPF_CFLAGS) -c $< -o $@
	$(AM_V_GEN)$(LLVM_STRIP) --strip-debug $@

if PROVIDED_VMLINUX_H
# Reproducible builds: use the vmlinux.h supplied at configure time via
# --with-vmlinux-h-path instead of reading the running kernel's BTF data.
vmlinux.h: $(VMLINUX_H_PATH) Makefile
	$(AM_V_GEN)cp $< $@
else
vmlinux.h: Makefile
	$(AM_V_GEN)$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@
endif

if BUILD_CAP_AUDIT
man_MANS = cap-audit.8
endif
