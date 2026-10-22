dnl
define([AC_INIT_NOTICE],
[### Generated automatically using autoconf version] AC_ACVERSION [
### Copyright 2009-2026 Steve Grubb <sgrubb@redhat.com>
###
### Permission is hereby granted, free of charge, to any person obtaining a
### copy of this software and associated documentation files (the "Software"),
### to deal in the Software without restriction, including without limitation
### the rights to use, copy, modify, merge, publish, distribute, sublicense,
### and/or sell copies of the Software, and to permit persons to whom the
### Software is furnished to do so, subject to the following conditions:
###
### The above copyright notice and this permission notice shall be included
### in all copies or substantial portions of the Software.
###
### THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
### IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
### FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
### THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
### OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
### ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
### OTHER DEALINGS IN THE SOFTWARE.
###
### For usage, run `./configure --help'
### For more detailed information on installation, read the file `INSTALL'.
###
### If configuration succeeds, status is in the file `config.status'.
### A log of configuration tests is in `config.log'.
])

AC_REVISION($Revision: 1.3 $)dnl
AC_INIT([libcap-ng],[0.9.6])
AC_PREREQ([2.50])dnl
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

echo Configuring libcap-ng $VERSION

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
LT_INIT
AC_SUBST(LIBTOOL_DEPS)

PKG_PROG_PKG_CONFIG

echo .
echo Checking for programs

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK

AC_MSG_CHECKING([for __atomic_load_n])
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM(
		[[unsigned int test_atomic;]],
		[[return __atomic_load_n(&test_atomic, __ATOMIC_ACQUIRE);]])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_FAILURE([Compiler support for __atomic_load_n is required by libcap-ng])])

LIBBPF_CFLAGS=""
LIBBPF_LIBS=""
LIBAUDIT_CFLAGS=""
LIBAUDIT_LIBS=""
CLANG=""
BPFTOOL=""
BPF_ARCH=""
AC_ARG_VAR([LLVM_STRIP], [llvm-strip command for cap-audit BPF objects])

AC_CANONICAL_HOST
AC_ARG_WITH([bpf-arch],
AS_HELP_STRING([--with-bpf-arch=ARCH],
[override detected BPF target arch (x86, arm64, arm, powerpc, s390, riscv, mips, loongarch)]),
[BPF_ARCH="$withval"])

AC_ARG_ENABLE([cap-audit],
AS_HELP_STRING([--enable-cap-audit],[build cap-audit [[default=no]]]),
[enable_cap_audit=$enableval],
[enable_cap_audit=no])

dnl For reproducible builds cap-audit's vmlinux.h can be supplied at
dnl configure time instead of being generated from the running kernel's BTF
dnl data in /sys/kernel/btf/vmlinux.
AC_ARG_WITH([vmlinux-h],
AS_HELP_STRING([--with-vmlinux-h=MODE],
[how to obtain cap-audit's vmlinux.h: auto, provided, or generated [[default=auto]]]),
[with_vmlinux_h="$withval"],
[with_vmlinux_h=auto])
AC_ARG_WITH([vmlinux-h-path],
AS_HELP_STRING([--with-vmlinux-h-path=PATH],
[path to a pre-generated vmlinux.h to use for cap-audit]),
[with_vmlinux_h_path="$withval"],
[with_vmlinux_h_path=""])

VMLINUX_H_PATH=""
vmlinux_h_status="n/a (cap-audit disabled)"

if test "x$enable_cap_audit" = "xyes"; then
missing_cap_audit_deps=no
AC_CHECK_PROG([CLANG],[clang],[clang],[no])
AC_CHECK_PROG([BPFTOOL],[bpftool],[bpftool],[no])
AC_CHECK_PROG([LLVM_STRIP],[llvm-strip],[llvm-strip],[no])
PKG_CHECK_MODULES([LIBBPF],[libbpf],
[have_libbpf=yes],[have_libbpf=no])
PKG_CHECK_MODULES([LIBAUDIT],[audit],
[have_libaudit=yes],[have_libaudit=no])
AC_CHECK_HEADERS([bpf/libbpf.h bpf/bpf.h linux/bpf.h libaudit.h], [],
[missing_cap_audit_deps=yes])
if test "$CLANG" = "no" -o "$BPFTOOL" = "no" -o "$LLVM_STRIP" = "no" -o \
"x$have_libbpf" != "xyes" -o "x$have_libaudit" != "xyes" -o \
"x$missing_cap_audit_deps" = "xyes"; then
AC_MSG_ERROR([cap-audit requires clang, llvm-strip, bpftool, libbpf, and libaudit])
fi

AC_MSG_CHECKING([how to obtain vmlinux.h])
case "$with_vmlinux_h" in
auto)
if test "x$with_vmlinux_h_path" != "x"; then
vmlinux_h_source=provided
else
vmlinux_h_source=generated
fi
;;
provided)
vmlinux_h_source=provided
;;
generated)
vmlinux_h_source=generated
;;
*)
AC_MSG_RESULT([error])
AC_MSG_ERROR([invalid --with-vmlinux-h value '$with_vmlinux_h' (expected auto, provided, or generated)])
;;
esac

if test "x$vmlinux_h_source" = "xprovided"; then
if test "x$with_vmlinux_h_path" = "x"; then
AC_MSG_RESULT([error])
AC_MSG_ERROR([--with-vmlinux-h=provided requires --with-vmlinux-h-path=PATH])
fi
if test ! -f "$with_vmlinux_h_path"; then
AC_MSG_RESULT([error])
AC_MSG_ERROR([provided vmlinux.h not found: $with_vmlinux_h_path])
fi
dnl Resolve to an absolute path so the build works from any directory.
case "$with_vmlinux_h_path" in
/*) VMLINUX_H_PATH="$with_vmlinux_h_path" ;;
*)  VMLINUX_H_PATH="`pwd`/$with_vmlinux_h_path" ;;
esac
vmlinux_h_status="provided ($VMLINUX_H_PATH)"
AC_MSG_RESULT([provided ($VMLINUX_H_PATH)])
else
vmlinux_h_status="generated from /sys/kernel/btf/vmlinux"
AC_MSG_RESULT([generated from /sys/kernel/btf/vmlinux])
fi
fi

if test "x$BPF_ARCH" = "x"; then
case "$host_cpu" in
x86_64|i?86)
BPF_ARCH=x86
;;
aarch64|arm64)
BPF_ARCH=arm64
;;
arm*)
BPF_ARCH=arm
;;
ppc64le|ppc64|powerpc*)
BPF_ARCH=powerpc
;;
s390x|s390)
BPF_ARCH=s390
;;
riscv64|riscv32|riscv*)
BPF_ARCH=riscv
;;
mips64|mips64el|mips|mipsel)
BPF_ARCH=mips
;;
loongarch64)
BPF_ARCH=loongarch
;;
*)
BPF_ARCH=x86
;;
esac
fi

if test "x$BPF_ARCH" = "x"; then
AC_MSG_ERROR([failed to determine BPF target arch; use --with-bpf-arch])
fi

AC_MSG_CHECKING(enable-deprecated)
AC_ARG_ENABLE([deprecated],
AS_HELP_STRING([--enable-deprecated],
[enable deprecated utilities such as captest [[default=no]]]),
[enable_deprecated=$enableval],
[enable_deprecated=no])
if test x$enable_deprecated = xno ; then
	AC_MSG_RESULT(no)
else
	AC_MSG_RESULT(yes)
fi

AC_SUBST(BPF_ARCH)
AC_SUBST(CLANG)
AC_SUBST(BPFTOOL)
AC_SUBST(LIBBPF_CFLAGS)
AC_SUBST(LIBBPF_LIBS)
AC_SUBST(LIBAUDIT_CFLAGS)
AC_SUBST(LIBAUDIT_LIBS)
AC_SUBST(VMLINUX_H_PATH)

AM_CONDITIONAL([BUILD_CAP_AUDIT], [test "x$enable_cap_audit" = "xyes"])
AM_CONDITIONAL([PROVIDED_VMLINUX_H], [test "x$VMLINUX_H_PATH" != "x"])
AM_CONDITIONAL([BUILD_DEPRECATED], [test "x$enable_deprecated" = "xyes"])

echo .
echo Checking for header files
AC_CHECK_HEADERS(linux/capability.h, [], [AC_MSG_ERROR(linux/capability.h is required in order to build libcap-ng.)])
AC_CHECK_HEADERS(sys/xattr.h, [], [
	AC_CHECK_HEADERS(attr/xattr.h, [], [AC_MSG_WARN(attr/xattr.h not found, disabling file system capabilities.)])
	])
AC_CHECK_HEADERS(linux/securebits.h, [], [])
AC_CHECK_HEADERS(bluetooth/bluetooth.h bluetooth/hci.h, [], [])
AC_CHECK_HEADERS(linux/inet_diag.h linux/netlink.h linux/sock_diag.h, [], [])
AC_CHECK_HEADERS(sys/socket.h, [], [])
AC_CHECK_HEADERS(linux/vm_sockets.h, [], [], [AC_INCLUDES_DEFAULT
		#ifdef HAVE_SYS_SOCKET_H
		#include <sys/socket.h>
		#endif
		])
AC_CHECK_HEADERS(linux/vm_sockets_diag.h, [], [])
AC_CHECK_HEADERS(pthread.h,
	[AC_SEARCH_LIBS(pthread_atfork, pthread)],
	[AC_MSG_WARN(pthread.h not found, disabling pthread_atfork.)])
AC_CHECK_HEADERS(sys/vfs.h, [
	AC_CHECK_HEADERS(linux/magic.h, [], [AC_MSG_WARN(linux/magic.h is required in order to verify procfs.)])
	], [AC_MSG_WARN(sys/vfs.h is required in order to verify procfs.)])

build_netcap_advanced=yes
netcap_advanced_missing=""
for netcap_hdr in \
	linux/inet_diag.h:$ac_cv_header_linux_inet_diag_h \
	linux/netlink.h:$ac_cv_header_linux_netlink_h \
	linux/sock_diag.h:$ac_cv_header_linux_sock_diag_h; do
	netcap_name=${netcap_hdr%%:*}
	netcap_have=${netcap_hdr#*:}
	AS_IF([test "x$netcap_have" != xyes], [
		build_netcap_advanced=no
		AS_IF([test "x$netcap_advanced_missing" = x], [
			netcap_advanced_missing="$netcap_name"
		], [
			netcap_advanced_missing="$netcap_advanced_missing, $netcap_name"
		])
	])
done

AS_IF([test "x$build_netcap_advanced" = xyes], [
	AC_DEFINE([HAVE_NETCAP_ADVANCED], [1],
		[Define to 1 if netcap advanced mode can be built.])
	netcap_advanced_status="enabled"
	AC_MSG_NOTICE([netcap advanced mode enabled])
], [
	netcap_advanced_status="disabled (missing headers: $netcap_advanced_missing)"
	AC_MSG_WARN([netcap advanced mode disabled; missing headers: $netcap_advanced_missing])
])
AC_SUBST([NETCAP_ADVANCED_ENABLED], [$build_netcap_advanced])
AC_SUBST([NETCAP_ADVANCED_STATUS], [$netcap_advanced_status])
AM_CONDITIONAL([BUILD_NETCAP_ADVANCED],
	[test "x$build_netcap_advanced" = xyes])

AS_IF([test "x$build_netcap_advanced" = xyes], [
	AS_IF([test "x$ac_cv_header_linux_vm_sockets_h" = xyes], [
		AC_DEFINE([HAVE_NETCAP_VSOCK], [1],
			[Define to 1 if netcap VSOCK support can be built.])
		netcap_vsock_status="enabled"
	], [
		netcap_vsock_status="disabled (missing header: linux/vm_sockets.h)"
		AC_MSG_WARN([netcap VSOCK support disabled; missing header: linux/vm_sockets.h])
	])
], [
	netcap_vsock_status="disabled (advanced mode disabled)"
])

AC_ARG_WITH([capability_header],
        [AS_HELP_STRING([--with-capability_header=path : path to capability.h])],
        [CAPABILITY_HEADER=$withval],
        [CAPABILITY_HEADER=/usr/include/linux/capability.h])
AC_SUBST(CAPABILITY_HEADER)

AC_MSG_CHECKING(__attr_dealloc_free support)
AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE(
    [[
      #include <stdio.h>
      extern char *test(const char *name) __attr_dealloc_free;
      int main(void) { return 0; }]])],
  [DEALLOC="yes"],
  [DEALLOC="no"]
)
AC_MSG_RESULT($DEALLOC)

AC_ARG_WITH(warn,
[  --with-warn             turn on warnings [[default=yes]]],
[
if test "x${withval}" = xyes; then
        WARNS="$ALLWARNS"
else
        WARNS=""
fi
],WARNS="$ALLWARNS")
AC_SUBST(DEBUG)

WFLAGS=""
AC_MSG_CHECKING(for -Wformat-truncation)
TMPCFLAGS="${CFLAGS}"
CFLAGS="${CFLAGS} -Wformat-truncation"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[WFLAGS="-Wno-format-truncation"
	AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)])
CFLAGS="${TMPCFLAGS}"
AC_SUBST(WFLAGS)

AC_CHECK_HEADER(sys/syscall.h,
	[AC_DEFINE([HAVE_SYSCALL_H], [1],
	[Define to 1 if you have <sys/syscall.h>.])], [],)

dnl; This is to record in the build logs what options are being taken
AC_CHECK_DECLS([PR_CAP_AMBIENT], [], [], [[#include <sys/prctl.h>]])
AC_CHECK_DECLS([VFS_CAP_REVISION_2], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([VFS_CAP_REVISION_3], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_EPOLLWAKEUP], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_WAKE_ALARM], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_BLOCK_SUSPEND], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_AUDIT_READ], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_PERFMON], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_BPF], [], [], [[#include <linux/capability.h>]])
AC_CHECK_DECLS([CAP_CHECKPOINT_RESTORE], [], [], [[#include <linux/capability.h>]])

dnl only build libdrop_ambient if support for ambient capabilities was found (which is normal)
if test x"${ac_cv_have_decl_PR_CAP_AMBIENT}" = x"no" ; then
    AC_MSG_WARN("PR_CAP_AMBIENT not available - libdrop_ambient will not be built")
fi
AM_CONDITIONAL(BUILD_LIBDROP_AMBIENT, test x"${ac_cv_have_decl_PR_CAP_AMBIENT}" = x"yes")

dnl Python 2 is no longer supported, so the generic Python option controls
dnl the Python 3 bindings. A version-specific option takes precedence because
dnl it is processed after the generic option.
AC_ARG_WITH(python,
AS_HELP_STRING([--with-python],
	[enable Python 3 bindings (use --without-python to disable)]),
use_python3=$withval,
use_python3=auto)

AC_MSG_CHECKING(whether to create python3 bindings)
AC_ARG_WITH(python3,
AS_HELP_STRING([--with-python3],[enable Python 3 bindings]),
use_python3=$withval)
AC_ARG_VAR([PYTHON3_CONFIG], [path to the Python 3 configuration helper])

python3_failure=
if test x$use_python3 = xno ; then
	AC_MSG_RESULT(no)
	swig_found=no
else
	AC_MSG_RESULT(investigating)
	AC_CHECK_PROG(swig_found, swig, yes, no)
	if test x$swig_found = xno ; then
		python3_failure="SWIG was not found"
	fi

	if test -z "$python3_failure" ; then
		AM_PATH_PYTHON([3.1], [],
			[python3_failure="a usable Python 3 interpreter was not found"])
	fi

	if test -z "$python3_failure" ; then
		dnl Prefer the configuration helper installed beside the selected
		dnl interpreter. The versioned fallback avoids mixing installations
		dnl when several Python 3 versions are present in PATH.
		if test -n "$PYTHON3_CONFIG" ; then
			AC_MSG_CHECKING([for a Python 3 configuration helper])
			AC_MSG_RESULT([$PYTHON3_CONFIG])
		elif test -x "${PYTHON}-config" ; then
			PYTHON3_CONFIG="${PYTHON}-config"
			AC_MSG_CHECKING([for a Python 3 configuration helper])
			AC_MSG_RESULT([$PYTHON3_CONFIG])
		else
			AC_PATH_PROGS([PYTHON3_CONFIG],
				[python${PYTHON_VERSION}-config python3-config], [no])
		fi
		if test x$PYTHON3_CONFIG = xno ; then
			python3_failure="a Python 3 configuration helper was not found"
		fi
	fi

	if test -z "$python3_failure" ; then
		AC_MSG_CHECKING([whether $PYTHON3_CONFIG provides build flags])
		if PYTHON3_CFLAGS=`"$PYTHON3_CONFIG" --cflags 2>/dev/null` &&
		   PYTHON3_INCLUDES=`"$PYTHON3_CONFIG" --includes 2>/dev/null` &&
		   PYTHON3_LIBS=`"$PYTHON3_CONFIG" --libs 2>/dev/null` ; then
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
			python3_failure="$PYTHON3_CONFIG could not provide build flags"
		fi
	fi

	if test -z "$python3_failure" ; then
		python3_major=`"$PYTHON" -c \
			'import sys; print(sys.version_info.major)'`
		python3_minor=`"$PYTHON" -c \
			'import sys; print(sys.version_info.minor)'`
		AC_MSG_CHECKING([whether Python.h matches $PYTHON])
		save_CPPFLAGS=$CPPFLAGS
		CPPFLAGS="$CPPFLAGS $PYTHON3_INCLUDES"
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <Python.h>
#if PY_MAJOR_VERSION != $python3_major || \
	PY_MINOR_VERSION != $python3_minor
#error Python.h does not match the selected interpreter
#endif]], [[]])],
			[AC_MSG_RESULT(yes)],
			[AC_MSG_RESULT(no)
			 python3_failure="Python.h is missing or does not match $PYTHON"])
		CPPFLAGS=$save_CPPFLAGS
	fi

	if test -n "$python3_failure" ; then
		if test x$use_python3 = xyes ; then
			AC_MSG_ERROR([$python3_failure])
		fi
		AC_MSG_NOTICE([$python3_failure - Python 3 bindings will not be built])
		use_python3=no
	else
		AC_MSG_NOTICE([Python 3 bindings will be built])
		use_python3=yes
	fi
fi
AC_SUBST(PYTHON3_CONFIG)
AC_SUBST(PYTHON3_CFLAGS)
AC_SUBST(PYTHON3_LIBS)
AC_SUBST(PYTHON3_INCLUDES)
AM_CONDITIONAL(HAVE_SWIG, test x"${swig_found}" = x"yes")
AM_CONDITIONAL(USE_PYTHON3, test ${use_python3} = "yes")

AC_CONFIG_FILES([Makefile src/Makefile src/libcap-ng.pc src/test/Makefile
bindings/Makefile
bindings/python3/Makefile bindings/src/Makefile
bindings/python3/test/Makefile bindings/test/Makefile utils/Makefile
utils/test/Makefile utils/cap-audit/Makefile \
utils/cap-audit/test/Makefile m4/Makefile docs/Makefile])
AC_CONFIG_COMMANDS([utils/netcap.8], [
	if test "x$NETCAP_ADVANCED_ENABLED" = xyes; then
		sed '/^.\\" BEGIN_ADVANCED$/d; /^.\\" END_ADVANCED$/d' \
			"$srcdir/utils/netcap.8.in" > utils/netcap.8
	else
		sed '/^.\\" BEGIN_ADVANCED$/,/^.\\" END_ADVANCED$/d' \
			"$srcdir/utils/netcap.8.in" > utils/netcap.8
	fi
], [NETCAP_ADVANCED_ENABLED="$NETCAP_ADVANCED_ENABLED"
	srcdir="$srcdir"])
AC_OUTPUT

echo .
echo "

  libcap-ng Version:      $VERSION
  Target:                 $target
  Installation prefix:    $prefix
  Build Kernel:           `uname -r`
  Compiler:               $CC
  Compiler flags:
`echo $CFLAGS | fmt -w 50 | sed 's,^,                          ,'`
  __attr_dealloc_free support:  $DEALLOC
  netcap advanced mode:  $netcap_advanced_status
  netcap VSOCK support:  $netcap_vsock_status
  cap-audit vmlinux.h:   $vmlinux_h_status
  "
