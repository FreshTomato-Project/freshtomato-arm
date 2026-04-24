# Continuous Integration

This directory contains scripts and other files used by the continuous integration
system.

# Running CI manually

It is possible to run parts of the CI pipeline manually. The best way to do this is to
start with a fresh virtual machine or container image of the desired operating system,
then to copy or mount the tinc source code directory into it, and then to call the
scripts from this directory inside the image. In particular:

- `./deps.sh` installs the required dependencies. It will automatically detect the
  operating system and calls the package manager to install the required support
  packages.
- `./build.sh <builddir>` builds tinc in the given build directory.
- `./test/prepare.sh` prepares the image for running tests with root privileges.
- `./test/run.sh <flavor>` runs the tests. You have to specify which flavor of tests:
  - `default`: tinc compiled with defaults.
  - `nolegacy`: tinc compiled without support for legacy cryptography.
  - `gcrypt`: tinc linked with libgcrypt instead of OpenSSL.

## Containers

Use Podman or Docker to create containers. For example, assuming you are in the root of
the tinc source directory, you can run the following commands to compile tinc in an
Alpine container:

```
podman run -it --rm -v `pwd`:/tmp/tinc:ro alpine:latest
cd /tmp/tinc
.ci/deps.sh
.ci/build.sh /tmp/build
```

The `podman run` command starts a container using a vanilla alpine:latest image, the
`-it` option ensures we can interact with a shell in the container, `--rm` makes it
automatically remove the container once you exit the shell, and the `-v` option makes a
read-only mount of the tinc source directory into the container as `/tmp/tinc`. This
allows you to work outside of the container on the source and have the results directly
visible inside the container, while any commands run inside the container cannot make
any changes to the sources.

To avoid having to install the dependencies each time you start a fresh container, you
can create a `Containerfile` or `Dockerfile` that creates an image with the dependencies
pre-installed:

```
FROM alpine:latest
WORKDIR /tmp/tinc
RUN --mount=type=bind,ro,src=.,dst=/tmp/tinc .ci/deps.sh
CMD .ci/build.sh /tmp/build
```

To build it, run:

```
podman build -t tinc-alpine -f /path/to/Containerfile .
```

This builds the image using the `Containerfile`, and gives it the name `tinc-alpine`.
Afterwards, you can run the container like so, which automatically starts building tinc:

```
podman run --rm -v `pwd`:/tmp/tinc:ro tinc-alpine
```

If you want to get a shell instead, run:

```
podman run --it --rm -v `pwd`:/tmp/tinc:ro tinc-alpine /bin/sh
```

## Virtual machines

While containers work very well when running Linux containers, if you want to test tinc
on other operating systems, you might have to use virtual machines instead. The
principles are very similar to containers, but there is no easy tool to create a VM
image of an arbitrary operating system. It's recommended to use KVM and optionally a UI
like virt-manager to create a VM image from a USB or CD-ROM installation image of the
desired operating system, and then copy `.ci/deps.sh` into it and run it (if supported).
Then stop using that virtual machine directly, but instead create a new one using
copy-on-write of the image you just created to build tinc.
