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

The one caveat is that printing capabilities from python does not work. But
you can still manipulate capabilities, though.

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

If you want python bindings, add that option to the configure command. The cap-audit program has to be specifically enabled and defaults to not being built. There is also a spec file to use if you are on a rpm based distribution. To do that, run "make dist" instead of make in the above instructions. Then use the resulting tar file with the spec file.

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
As of the 0.9 release of libcap-ng, there is a new utility **cap-audit**. This program can be used to determine the actual capabilities that a program needs. To do this, use it to run the application kind of the way one would use strace. Use '--' to separate the options to cap-audit from the program being audited. You need to use cap-audit as root because it places an eBPF program in the kernel to hook the capability checks to determine what was requested, was it granted, and what syscall did it originate from. When testing a daemon, pass command line options that keep it in the foreground. The following is an example checking sshd: 

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

CONDITIONAL CAPABILITIES:
----------------------------------------------------------------------
  CAP_DAC_OVERRIDE
    Needed when fs.protected_symlinks = 1 for symlinks in world-writable directories
    Current value: 1 (capability needed)

ATTEMPTED BUT DENIED:
----------------------------------------------------------------------
  dac_override (#1)
    Attempts: 2 (all denied)
    Impact: Application may have reduced functionality

  bpf (#39)
    Attempts: 1 (all denied)
    Impact: Application may have reduced functionality

SUMMARY:
----------------------------------------------------------------------
  Total capability checks: 2349
  Required capabilities: 11
  Conditional capabilities: 1
  Denied operations: 2

RECOMMENDATIONS:
----------------------------------------------------------------------
  Programmatic solution (C with libcap-ng):
    #include <cap-ng.h>
    ...
    capng_clear(CAPNG_SELECT_BOTH);
    capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CHOWN, DAC_READ_SEARCH, SETGID, SETUID, NET_BIND_SERVICE, NET_ADMIN, SYS_CHROOT, SYS_ADMIN, SYS_RESOURCE, AUDIT_WRITE, MAC_ADMIN, -1);
    if (capng_change_id(uid, gid, CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING))
	perror("capng_change_id");

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
https://github.com/stevegrubb/libcap-ng/issue

