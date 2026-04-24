#!/bin/sh

set -eu

SKIP_MESON="${SKIP_MESON:-}"

deps_linux_alpine() {
  apk upgrade

  apk add \
    git pkgconf gcc linux-headers shadow libgcrypt-dev gzip iproute2 iputils-ping iputils-arping mount \
    openssl-dev zlib-dev lzo-dev ncurses-dev readline-dev musl-dev lz4-dev vde2-dev cmocka-dev \
    "$@"

  if [ -z "$SKIP_MESON" ]; then
    apk add meson
  fi
}

deps_linux_debian_mingw() {
  apt install --no-install-recommends -y \
    mingw-w64 mingw-w64-tools \
    wine wine-binfmt \
    libgcrypt-mingw-w64-dev \
    "$@"
}

deps_linux_debian_linux() {
  if [ -n "$HOST" ]; then
    dpkg --add-architecture "$HOST"
    apt update
  fi

  apt install --no-install-recommends -y \
    iproute2 iputils-ping iputils-arping \
    python3-cryptography \
    build-essential \
    binfmt-support binutils \
    zlib1g-dev:"$HOST" \
    libssl-dev:"$HOST" \
    liblzo2-dev:"$HOST" \
    liblz4-dev:"$HOST" \
    libncurses-dev:"$HOST" \
    libreadline-dev:"$HOST" \
    libgcrypt20-dev:"$HOST" \
    libminiupnpc-dev:"$HOST" \
    libvdeplug-dev:"$HOST" \
    libcmocka-dev:"$HOST" \
    pkgconf:"$HOST" \
    "$@"

  apt install --no-install-recommends systemd-dev ||
    apt install --no-install-recommends libsystemd-dev

  if [ -n "$HOST" ]; then
    apt install --no-install-recommends -y crossbuild-essential-"$HOST" qemu-user qemu-user-binfmt
  fi
}

deps_linux_debian() {
  export DEBIAN_FRONTEND=noninteractive

  apt update
  apt upgrade -y
  apt install --no-install-recommends -y git pkgconf texinfo

  HOST=${HOST:-}
  if [ "$HOST" = mingw ]; then
    deps_linux_debian_mingw "$@"
  else
    deps_linux_debian_linux "$@"
  fi

  if [ -z "$SKIP_MESON" ]; then
    apt install -y meson
  fi

  . /etc/os-release
}

deps_linux_rhel() {
  yum upgrade -y

  if [ "$ID" != fedora ]; then
    yum install -y epel-release

    if type dnf; then
      dnf install -y 'dnf-command(config-manager)'
      dnf config-manager --enable powertools || true
      dnf config-manager --enable crb || true
    fi
  fi

  yum install -y \
    git pkgconf gcc \
    lzo-devel zlib-devel lz4-devel ncurses-devel readline-devel libgcrypt-devel systemd-devel \
    libcmocka-devel cmake \
    openssl-devel "$@"

  if [ -z "$SKIP_MESON" ]; then
    yum install -y meson
  fi

  if yum info miniupnpc-devel; then
    yum install -y miniupnpc-devel
  fi
}

deps_linux() {
  . /etc/os-release

  case "$ID" in
  alpine)
    deps_linux_alpine "$@"
    ;;

  debian | ubuntu)
    deps_linux_debian "$@"
    ;;

  fedora | centos | almalinux)
    deps_linux_rhel "$@"
    ;;

  *) exit 1 ;;
  esac
}

deps_macos() {
  brew install lzo lz4 miniupnpc libgcrypt openssl "$@"

  if [ -z "$SKIP_MESON" ]; then
    brew install meson
  fi
}

case "$(uname -s)" in
Linux) deps_linux "$@" ;;
Darwin) deps_macos "$@" ;;
*) exit 1 ;;
esac
