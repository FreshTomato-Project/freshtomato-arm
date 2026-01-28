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

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src ${LIBBPF_CFLAGS} ${LIBAUDIT_CFLAGS}
AM_CFLAGS = -W -Wall -Wshadow ${WFLAGS} -Wundef -D_GNU_SOURCE

bin_PROGRAMS = cap-audit
cap_audit_SOURCES = cap_audit.c
nodist_cap_audit_SOURCES = cap_audit.skel.h
cap_audit_LDADD = ${top_builddir}/src/libcap-ng.la ${LIBBPF_LIBS} ${LIBAUDIT_LIBS}

EXTRA_DIST = cap_audit.bpf.c cap-audit.8
CLEANFILES = cap_audit.bpf.o cap_audit.skel.h vmlinux.h

if BUILD_CAP_AUDIT
BUILT_SOURCES = cap_audit.bpf.o cap_audit.skel.h vmlinux.h
endif

BPF_ARCH = @BPF_ARCH@
BPFTOOL = @BPFTOOL@
CLANG = @CLANG@

BPF_CFLAGS = -g -O2 -target bpf -D__TARGET_ARCH_${BPF_ARCH} ${AM_CPPFLAGS}

cap_audit.skel.h: cap_audit.bpf.o
	$(AM_V_GEN)$(BPFTOOL) gen skeleton $< > $@

cap_audit.bpf.o: cap_audit.bpf.c vmlinux.h
	$(AM_V_CC)$(CLANG) $(BPF_CFLAGS) -c $< -o $@

vmlinux.h:
	$(AM_V_GEN)$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

man_MANS = cap-audit.8
