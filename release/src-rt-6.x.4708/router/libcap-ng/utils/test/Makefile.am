# SPDX-License-Identifier: GPL-2.0-or-later
# Makefile.am -- utility tests
#
# Copyright 2026 Red Hat Inc.
# All Rights Reserved.

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src -I${top_srcdir}/utils \
	-I${top_srcdir}/utils/cap-audit

check_PROGRAMS = utils_test utility_logic_test pscap_test
TESTS = $(check_PROGRAMS)

utils_test_SOURCES = utils_test.c \
	${top_srcdir}/utils/filecap-path.c \
	${top_srcdir}/utils/proc-account.c \
	${top_srcdir}/utils/proc-status.c

utility_logic_test_SOURCES = utility_logic_test.c \
	${top_srcdir}/utils/netcap.c \
	${top_srcdir}/utils/proc-account.c \
	${top_srcdir}/utils/proc-llist.c \
	${top_srcdir}/utils/proc-output.c \
	${top_srcdir}/utils/proc-sanitize.c
utility_logic_test_CPPFLAGS = $(AM_CPPFLAGS) -DNETCAP_NO_MAIN -DNETCAP_TEST
utility_logic_test_LDADD = ${top_builddir}/src/libcap-ng.la

pscap_test_SOURCES = pscap_test.c \
	${top_srcdir}/utils/proc-account.c \
	${top_srcdir}/utils/proc-output.c \
	${top_srcdir}/utils/proc-sanitize.c \
	${top_srcdir}/utils/proc-status.c
pscap_test_CPPFLAGS = $(AM_CPPFLAGS) -D_GNU_SOURCE
pscap_test_LDADD = ${top_builddir}/src/libcap-ng.la

if BUILD_NETCAP_ADVANCED
check_PROGRAMS += netcap_advanced_test
netcap_advanced_test_SOURCES = netcap_advanced_test.c \
	${top_srcdir}/utils/proc-output.c \
	${top_srcdir}/utils/proc-sanitize.c \
	${top_srcdir}/utils/proc-status.c
netcap_advanced_test_CPPFLAGS = $(AM_CPPFLAGS) -D_GNU_SOURCE
netcap_advanced_test_LDADD = ${top_builddir}/src/libcap-ng.la
endif
