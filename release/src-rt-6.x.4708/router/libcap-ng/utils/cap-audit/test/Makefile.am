# SPDX-License-Identifier: GPL-2.0-or-later
# Makefile.am -- cap-audit tests
#
# Copyright 2026 Red Hat Inc.
# All Rights Reserved.

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src \
	-I${top_srcdir}/utils \
	-I${top_srcdir}/utils/cap-audit \
	-I${top_builddir}/utils/cap-audit ${LIBBPF_CFLAGS}

check_PROGRAMS = cap_audit_util_test cap_audit_path_test
TESTS = $(check_PROGRAMS)

cap_audit_util_test_SOURCES = cap_audit_util_test.c \
	${top_srcdir}/utils/cap-audit/classify_app.c

cap_audit_path_test_SOURCES = cap_audit_path_test.c \
	${top_srcdir}/utils/cap-audit/cap_audit_path.c

if BUILD_CAP_AUDIT
check_PROGRAMS += cap_audit_namespace_test cap_audit_outcome_test \
	cap_audit_recommendation_test cap_audit_service_test
cap_audit_namespace_test_SOURCES = cap_audit_namespace_test.c \
	${top_srcdir}/utils/cap-audit/cap_audit_event.c \
	${top_srcdir}/utils/cap-audit/cap_audit_analyze.c \
	${top_srcdir}/utils/cap-audit/cap_audit_capset.c \
	${top_srcdir}/utils/cap-audit/cap_audit_outcome.c
cap_audit_outcome_test_SOURCES = cap_audit_outcome_test.c \
	${top_srcdir}/utils/cap-audit/cap_audit_event.c \
	${top_srcdir}/utils/cap-audit/cap_audit_analyze.c \
	${top_srcdir}/utils/cap-audit/cap_audit_capset.c \
	${top_srcdir}/utils/cap-audit/cap_audit_json.c \
	${top_srcdir}/utils/cap-audit/cap_audit_outcome.c \
	${top_srcdir}/utils/cap-audit/cap_audit_yaml.c
cap_audit_outcome_test_LDADD = ${top_builddir}/src/libcap-ng.la
cap_audit_recommendation_test_SOURCES = cap_audit_recommendation_test.c \
	${top_srcdir}/utils/cap-audit/cap_audit_analyze.c \
	${top_srcdir}/utils/cap-audit/cap_audit_capset.c \
	${top_srcdir}/utils/cap-audit/cap_audit_outcome.c
cap_audit_service_test_SOURCES = cap_audit_service_test.c \
	${top_srcdir}/utils/cap-audit/cap_audit_service.c
cap_audit_service_test_LDADD = ${top_builddir}/src/libcap-ng.la \
	${LIBBPF_LIBS}
cap_audit_service_test_LDFLAGS = -Wl,--wrap=capng_change_id \
	-Wl,--wrap=capng_get_caps_process
endif
