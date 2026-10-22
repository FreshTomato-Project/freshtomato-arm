libcap-ng
=========

The libcap-ng library should make programming with POSIX capabilities
easier. The library has some utilities to help you analyze a system
for apps that may have too much privileges.

The included utilities are designed to let admins and developers spot apps from various ways that may be running with too much privilege. For example, any investigation should start with network facing apps since they would be prime targets for intrusion. The **netcap** program will check all running apps that have listening socket and display the results. Sample output from netcap:

```
ppid  pid   acct       command          type port  capabilities
1     2295  root       nasd             tcp  8000  full
2323  2383  root       dnsmasq          tcp  53    net_admin, net_raw +
1     2286  root       sshd             tcp  22    full
1     2365  root       cupsd            tcp  631   full
1     2286  root       sshd             tcp6 22    full
1     2365  root       cupsd            tcp6 631   full
2323  2383  root       dnsmasq          udp  53    net_admin, net_raw +
2323  2383  root       dnsmasq          udp  67    net_admin, net_raw +
1     2365  root       cupsd            udp  631   full
```
After checking the networking apps, you should check all running apps with
**pscap**. If you are a developer and have to give your application
CAP_DAC_OVERRIDE, you must be accessing files for which you have no permission
to access. This typically can be resolved by having membership in the correct
groups. Try to avoid needing CAP_DAC_OVERRIDE...you may as well be root if you
need it.

Some application developers have chosen to use file system based capabilities
rather than be setuid root and have to drop capabilities. Libcap-ng provides
**filecap** to recursively search directories and show you which ones have
capabilities and exactly what those are.

There is a new utility, **cap-audit** which can audit a program for necessary capabilities. It is discussed in more detail below.

C Examples
----------
As an application developer, there are probably 6 use cases that you are
interested in: drop all capabilities, keep one capability, keep several
capabilities, check if you have any capabilities at all, check for certain
capabilities, and retain capabilities across a uid change.

1) Drop all capabilities
   ```c
   capng_clear(CAPNG_SELECT_BOTH);
   capng_apply(CAPNG_SELECT_BOTH);
   ```

2) Keep one capability
   ```c
   capng_clear(CAPNG_SELECT_BOTH);
   capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_CHOWN);
   capng_apply(CAPNG_SELECT_BOTH);
   ```

3) Keep several capabilities
   ```c
   capng_clear(CAPNG_SELECT_BOTH);
   capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_SETUID, CAP_SETGID, -1);
   capng_apply(CAPNG_SELECT_BOTH);
   ```

4) Check if you have any capabilities
   ```c
   if (capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE)
       do_something();
   ```

5) Check for a specific capability
   ```c
   if (capng_have_capability(CAPNG_EFFECTIVE, CAP_CHOWN))
       do_something();
   ```

6) Retain capabilities across a uid change
   ```c
   capng_clear(CAPNG_SELECT_BOTH);
   capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_CHOWN);
   if (capng_change_id(99, 99, CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING))
       error();
   ```

Now, isn't that a lot simpler? Note that the last example takes about 60 lines
of code using the older capabilities library. As of the 0.6 release, there is
a m4 macro file to help add libcap-ng to your autotools config system. In
configure.ac, add LIBCAP_NG_PATH. Then in Makefile.am locate the apps that
link to libcap-ng, add $(CAPNG_LDADD) to their LDADD entries. And lastly,
surround the optional capabilities code with #ifdef HAVE_LIBCAP_NG.

Python
------
Libcap-ng 0.6 and later has python bindings. (Only python3 is supported from 0.8.4 onward.) You simply add 'import capng' in your script.  Here are the same examples as above in python:

1) Drop all capabilities
   ```python
   capng.capng_clear(capng.CAPNG_SELECT_BOTH)
   capng.capng_apply(capng.CAPNG_SELECT_BOTH)
   ```

2) Keep one capability
   ```python
   capng.capng_clear(capng.CAPNG_SELECT_BOTH)
   capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_CHOWN)
   capng.capng_apply(capng.CAPNG_SELECT_BOTH)
   ```

3) Keep several capabilities
   ```python
   capng.capng_clear(capng.CAPNG_SELECT_BOTH)
   capng.capng_updatev(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_SETUID, capng.CAP_SETGID, -1)
   capng.capng_apply(capng.CAPNG_SELECT_BOTH)
   ```

4) Check if you have any capabilities
   ```python
   if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) > capng.CAPNG_NONE:
       do_something()
   ```

5) Check for a specific capability
   ```python
   if capng.capng_have_capability(capng.CAPNG_EFFECTIVE, capng.CAP_CHOWN):
       do_something()
   ```

6) Retain capabilities across a uid change
   ```python
   capng.capng_clear(capng.CAPNG_SELECT_BOTH)
   capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE|capng.CAPNG_PERMITTED, capng.CAP_CHOWN)
   if capng.capng_change_id(99, 99, capng.CAPNG_DROP_SUPP_GRP | capng.CAPNG_CLEAR_BOUNDING) < 0:
       error()
   ```

The Python bindings also provide `capng_print_caps_numeric()` and
`capng_print_caps_text()` for printing capabilities or returning them as a
Python string.

Ambient Capabilities
--------------------
Ambient capabilities arrived in the 4.3 Linux kernel. Ambient capabilities
allow a privileged process to bestow capabilities to a child process. This
is how systemd grants capabilities to a daemon running in a service account.
The problem with ambient capabilities is they are inherited forever. Every
process exec'ed from the original service also has the capabilities. This is
a security issue.

To find and fix this, you can run the pscap program and grep for '@'. The '@'
symbol denotes processes that have ambient capabilities. For example:

```
# pscap | grep @
1     1655  systemd-oom  systemd-oomd        dac_override, kill @ +
1     1656  systemd-resolve  systemd-resolve     net_raw @ +

```

To fix this, libcap-ng 0.8.3 and later ships libdrop_ambient.so.0. It is
designed to be used with LD_PRELOAD. It has a constructor function that forces
the dropping of ambient capabilities. By the time the application starts, it
has both effective and ambient capabilities - meaning is safe to drop ambient
capabilities very early. You can either link it to an application run as a
systemd service (using ld), or create a wrapper script that then starts the
daemon.

Building
--------

Note: As of the 0.9 release, libcap-ng is no longer being distributed from people.redhat.com/sgrubb/libcap-ng/ please adjust any scripts to watch this github page for new releases.

After cloning libcap-ng, run:

```
cd libcap-ng
./autogen.sh
./configure
make
make install
```

Use `--with-python` or `--with-python3` to enable the Python 3 bindings, or
`--without-python` to disable them. The `netcap --advanced` feature also
depends on newer Linux kernel headers. When
the core inet/netlink diag headers are not available, such as on older build
roots, `configure` will automatically opt out of advanced mode and report that
in its output while still building the rest of libcap-ng. VSOCK reporting
additionally depends on `linux/vm_sockets.h`; if only that header is missing,
`netcap --advanced` is still built and VSOCK endpoints are omitted from its
reports. The cap-audit program has to be specifically enabled and defaults to
not being built. There is also a spec file to use if you are on a rpm based
distribution. To do that, run "make dist" instead of make in the above
instructions. Then use the resulting tar file with the spec file.

When advanced mode is available, `netcap --advanced --list-interfaces` prints
the current network namespace interface names. Add `--json` to get a
machine-readable list of names.

NOTE: to distributions
----------------------
There is a "make check" target. It only works if the available kernel headers
roughly match the build root kernel. Iow, if you have a chroot build system
that is using a much older kernel, the macros in the kernel header files will
describe functionality that does not exist in the build root. The capng_init
function will probe the kernel and decide we can only do v1 rather than v3
capabilities instead of what the kernel headers said was possible. If that is
your case, just don't do the "make check" as part of the build process. This
problem should go away as build roots eventually switch to the 5.0 or later
kernels.

cap-audit
---------
As of the 0.9 release of libcap-ng, there is a new utility **cap-audit**. This
program can be used to determine the actual capabilities that a program needs.
To do this, use it to run the application kind of the way one would use
strace. Use `--` to separate the options to cap-audit from the program being
audited. Runtime tracing requires root or the `CAP_BPF`, `CAP_PERFMON`, and
`CAP_SYS_PTRACE` capabilities. When testing a daemon directly, pass command
line options that keep it in the foreground. The following is an example
checking sshd:

The `--service` option audits a systemd service unit. It parses and runs
`ExecStart` while simulating the unit's credentials, capability sets, and
`NoNewPrivileges` setting. By default, cap-audit traces the unit's `ExecStart`
command:

```
cap-audit --service /usr/lib/systemd/system/sshd.service
```

Arguments beginning with `-` after `--` replace the parsed `ExecStart`
arguments, while a complete command after `--` replaces `ExecStart` entirely:

```
cap-audit --service /usr/lib/systemd/system/sshd.service -- -D
cap-audit --service service-file -- /usr/sbin/sshd -D
```

This is a focused service simulator rather than a complete systemd execution
environment. In particular, it does not load `Environment` or
`EnvironmentFile`; see the cap-audit(8) man page for the supported settings
and `ExecStart` prefix limitations.

Cap-audit also records the capability masks installed by successful `capset`
calls. A capability requested by `capset` without a confirmed granted check is
reported separately under `CAPSET-ONLY CAPABILITIES`; it is not classified as
required. Deployment snippets retain these capabilities for compatibility with
the current binary because removing one from the deployment boundary can make
the application's `capset` fail even when the capability is no longer
functionally needed. Treat them as application-cleanup candidates: remove the
capability from the application's setup, exercise representative code paths in
additional runs, and only then remove it from the deployment boundary. Failed
`capset` calls do not establish this compatibility constraint.

cap-audit is optional and has to be enabled at build time:

```
./configure --enable-cap-audit
```

The build needs clang, llvm-strip, bpftool, libbpf, libaudit, and their
development headers. Set `LLVM_STRIP=/path/to/llvm-strip` when configuring
to select a versioned or non-default tool. The build strips DWARF debug
information from the BPF object before embedding it, while preserving the
BTF and CO-RE metadata needed to load it.

It also needs a `utils/cap-audit/vmlinux.h` describing the kernel's
types. By default this is generated from the running kernel's BTF data in
`/sys/kernel/btf/vmlinux`, so the build host kernel must provide enough BTF and
tracing metadata for the BPF program to compile.

Reading `/sys/kernel/btf/vmlinux` at build time makes the build depend on the
host kernel and is therefore not reproducible. To avoid this, a pre-generated
`vmlinux.h` can be supplied at configure time:

```
./configure --enable-cap-audit \
    --with-vmlinux-h=provided \
    --with-vmlinux-h-path=/path/to/vmlinux.h
```

`--with-vmlinux-h` selects how `vmlinux.h` is obtained:

* `auto` (default) uses the file given by `--with-vmlinux-h-path` if set, and
  otherwise generates one from `/sys/kernel/btf/vmlinux`.
* `provided` requires `--with-vmlinux-h-path` and uses that file as-is,
  without touching `/sys`.
* `generated` always generates the file from `/sys/kernel/btf/vmlinux`.

A `vmlinux.h` for the target architecture can be produced ahead of time with:

```
bpftool btf dump file /sys/kernel/btf/vmlinux format c >vmlinux.h
```

The kernel used to build and run cap-audit needs the BPF and tracing support
used by `utils/cap-audit/cap_audit.bpf.c`. On current kernels, check for these
options in `/proc/config.gz` or `/boot/config-$(uname -r)`:

```
CONFIG_BPF=y
CONFIG_BPF_SYSCALL=y
CONFIG_BPF_EVENTS=y
CONFIG_DEBUG_INFO_BTF=y
CONFIG_FTRACE_SYSCALLS=y
CONFIG_KPROBES=y
CONFIG_KRETPROBES=y
CONFIG_KPROBE_EVENTS=y
CONFIG_PERF_EVENTS=y
CONFIG_STACKTRACE=y
CONFIG_TRACEPOINTS=y
CONFIG_TRACING=y
```

`CONFIG_FTRACE_SYSCALLS` is needed for the `raw_syscalls/sys_enter` and
`raw_syscalls/sys_exit` tracepoint types. If it is missing, the generated
`vmlinux.h` may not contain `struct trace_event_raw_sys_enter` or
`struct trace_event_raw_sys_exit`, and `cap_audit.bpf.c` will not compile. A
quick check for the tracepoint structs used by cap-audit is:

```
bpftool btf dump file /sys/kernel/btf/vmlinux format c |
grep -E 'struct trace_event_raw_(sys_enter|sys_exit|sched_process_(fork|exec|template))'
```

`CONFIG_BPF_JIT` is recommended for performance, but it is not required just to
build cap-audit.

```
cap-audit -- /usr/sbin/sshd -D
[*] Capability auditor started
[*] Tracing application: /usr/sbin/sshd (PID 30581)
[*] Press Ctrl-C to stop

^C[*] Analyzing results...

======================================================================
CAPABILITY ANALYSIS FOR: /usr/sbin/sshd (PID 30581)
======================================================================

SYSTEM CONTEXT:
----------------------------------------------------------------------
  Kernel version: 6.18.3-100.fc42.x86_64
  kernel.yama.ptrace_scope: 1
  kernel.kptr_restrict: 1
  kernel.dmesg_restrict: 1
  kernel.modules_disabled: 0
  kernel.perf_event_paranoid: 2
  kernel.unprivileged_bpf_disabled: 2
  net.core.bpf_jit_enable: 1
  net.core.bpf_jit_harden: 1
  net.core.bpf_jit_kallsyms: 1
  vm.mmap_min_addr: 65536
  fs.protected_hardlinks: 1
  fs.protected_symlinks: 1
  fs.suid_dumpable: 2

REQUIRED CAPABILITIES:
----------------------------------------------------------------------
  chown (#0)
    Checks: 1 granted, 0 denied
    Reason: Used by chown (syscall 92)

  dac_read_search (#2)
    Checks: 2 granted, 1 denied
    Reason: Used by openat (syscall 257)

  setgid (#6)
    Checks: 30 granted, 2 denied
    Reason: Used by setgroups (syscall 116)

  setuid (#7)
    Checks: 13 granted, 4 denied
    Reason: Used by setresuid (syscall 117)

  net_bind_service (#10)
    Checks: 2 granted, 0 denied
    Reason: Used by bind (syscall 49)

  net_admin (#12)
    Checks: 2 granted, 2 denied
    Reason: Used by setsockopt (syscall 54)

  sys_chroot (#18)
    Checks: 1 granted, 0 denied
    Reason: Used by chroot (syscall 161)

  sys_admin (#21)
    Checks: 597 granted, 1656 denied
    Reason: Used by brk (syscall 12)

  sys_resource (#24)
    Checks: 3 granted, 0 denied
    Reason: Used by write (syscall 1)

  audit_write (#29)
    Checks: 27 granted, 2 denied
    Reason: Used by sendto (syscall 44)

  mac_admin (#33)
    Checks: 1 granted, 0 denied
    Reason: Used by getxattr (syscall 191)

CAPSET-ONLY CAPABILITIES:
----------------------------------------------------------------------
  None

CONDITIONAL CAPABILITIES:
----------------------------------------------------------------------
  CAP_DAC_OVERRIDE
    Needed when fs.protected_symlinks = 1 for symlinks in world-writable directories
    Current value: 1 (capability needed)

CAPABILITIES WITH ONLY NOT-GRANTED CHECKS:
----------------------------------------------------------------------
  dac_override (#1)
    Capability checks returning not granted: 2
    Outcomes of syscall invocations containing such a check:
      readlink: 1 succeeded
      faccessat2: 1 succeeded
    Assessment: Associated syscall outcomes: 2 succeeded. The capability was not
                granted by those checks, but the associated syscall invocations
                completed. The capability is not automatically recommended.
    Recommendation: Not added automatically.

  bpf (#39)
    Capability checks returning not granted: 2
    Outcomes of syscall invocations containing such a check:
      prctl: 2 succeeded
    Assessment: Associated syscall outcomes: 2 succeeded. The capability was not
                granted by those checks, but the associated syscall invocations
                completed. The capability is not automatically recommended.
    Recommendation: Not added automatically.

SUMMARY:
----------------------------------------------------------------------
  Total capability checks: 2349
  Required capabilities: 11
  Conditional capabilities: 1
  Successful capset calls: 0
  Capset-only capabilities: 0
  Capabilities with only not-granted checks: 2
    Assessment counts:
      Omitted after associated syscall success: 2

RECOMMENDATIONS:
----------------------------------------------------------------------
  Capabilities with only not-granted checks:
    dac_override: Omitted; associated syscalls succeeded
    bpf: Omitted; associated syscalls succeeded
    Capabilities in this section are not added automatically. Add one only
    after confirming that required functionality fails because the capability
    is absent.

  Programmatic solution (C with libcap-ng):
    #include <cap-ng.h>
    #include <stdio.h>
    #include <stdlib.h>
    ...
    capng_clear(CAPNG_SELECT_BOTH);
    capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_CHOWN, CAP_DAC_READ_SEARCH, CAP_SETGID, CAP_SETUID, CAP_NET_BIND_SERVICE, CAP_NET_ADMIN, CAP_SYS_CHROOT, CAP_SYS_ADMIN, CAP_SYS_RESOURCE, CAP_AUDIT_WRITE, CAP_MAC_ADMIN, -1);
    if (capng_change_id(uid, gid, CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING)) {
	perror("capng_change_id");
	exit(EXIT_FAILURE);
    }

  For systemd service:
    [Service]
    User=<non-root-user>
    Group=<non-root-group>
    AmbientCapabilities=chown dac_read_search setgid setuid net_bind_service net_admin sys_chroot sys_admin sys_resource audit_write mac_admin
    CapabilityBoundingSet=chown dac_read_search setgid setuid net_bind_service net_admin sys_chroot sys_admin sys_resource audit_write mac_admin

  For file capabilities (via filecap):
    filecap /path/to/binary chown dac_read_search setgid setuid net_bind_service net_admin sys_chroot sys_admin sys_resource audit_write mac_admin

  For Docker/Podman:
    docker run --user $(id -u):$(id -g) \
      --cap-drop=ALL \
      --cap-add=chown \
      --cap-add=dac_read_search \
      --cap-add=setgid \
      --cap-add=setuid \
      --cap-add=net_bind_service \
      --cap-add=net_admin \
      --cap-add=sys_chroot \
      --cap-add=sys_admin \
      --cap-add=sys_resource \
      --cap-add=audit_write \
      --cap-add=mac_admin \
      your-image:tag

  For Kubernetes:
    securityContext:
      runAsUser: 1000
      runAsGroup: 1000
      capabilities:
        drop:
          - ALL
        add:
          - chown
          - dac_read_search
          - setgid
          - setuid
          - net_bind_service
          - net_admin
          - sys_chroot
          - sys_admin
          - sys_resource
          - audit_write
          - mac_admin
```

Reporting
---------
Report any bugs in this package to:
https://github.com/stevegrubb/libcap-ng/issues
