# Makefile.am --
# Copyright 2009,2015,2022,2024,2026 Red Hat Inc.
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
# Boston, MA 02110-1335, USA.
#
# Authors:
#   Steve Grubb <sgrubb@redhat.com>
# 

CONFIG_CLEAN_FILES = *.loT *.rej *.orig
DISTCLEANFILES = netcap.8
SUBDIRS = test cap-audit
DIST_SUBDIRS = test cap-audit
EXTRA_DIST = pscap.8 filecap.8 netcap.8.in captest.c captest.8 \
	libcap-ng.bash_completion
noinst_HEADERS = gcc-attributes.h
AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/src
LDADD = ${top_builddir}/src/libcap-ng.la
AM_CFLAGS = -W -Wall -Wshadow ${WFLAGS} -Wundef -D_GNU_SOURCE
NETCAP_ADVANCED_ENABLED = @NETCAP_ADVANCED_ENABLED@
bin_PROGRAMS = pscap netcap filecap
man_MANS = pscap.8 netcap.8 filecap.8
bashcompletiondir = ${datadir}/bash-completion/completions
bashcompletioncommands = cap-audit filecap netcap pscap

pscap_SOURCES = pscap.c proc-account.c proc-account.h \
	proc-output.c proc-output.h proc-sanitize.c proc-sanitize.h \
	proc-status.c proc-status.h
netcap_SOURCES = netcap.c netcap-advanced.h proc-llist.c \
	proc-llist.h proc-account.c proc-account.h proc-sanitize.c \
	proc-sanitize.h
if BUILD_NETCAP_ADVANCED
netcap_SOURCES += netcap-advanced.c proc-output.c proc-output.h \
	proc-status.c proc-status.h
endif
filecap_SOURCES = filecap.c filecap-path.c filecap-path.h

if BUILD_DEPRECATED
bin_PROGRAMS += captest
man_MANS += captest.8
captest_SOURCES = captest.c
endif

install-exec-hook:
	$(MKDIR_P) ${DESTDIR}${bashcompletiondir}
	$(INSTALL_DATA) ${srcdir}/libcap-ng.bash_completion \
		${DESTDIR}${bashcompletiondir}/libcap-ng.bash_completion
	cd ${DESTDIR}${bashcompletiondir} && \
	for command in $(bashcompletioncommands); do \
		$(LN_S) -f libcap-ng.bash_completion $$command || exit 1; \
	done

uninstall-hook:
	for command in $(bashcompletioncommands); do \
		rm -f ${DESTDIR}${bashcompletiondir}/$$command; \
	done
	rm -f ${DESTDIR}${bashcompletiondir}/libcap-ng.bash_completion
