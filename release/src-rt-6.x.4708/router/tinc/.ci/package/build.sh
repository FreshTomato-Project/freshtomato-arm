#!/bin/sh

set -eu

build_linux() {
  . /etc/os-release

  # https://github.com/actions/checkout/issues/760
  git config --global --add safe.directory "$PWD" || true
  GIT_CEILING_DIRECTORIES=$PWD
  export GIT_CEILING_DIRECTORIES

  case "$ID" in
  debian | ubuntu)
    .ci/package/deb/build.sh
    ;;
  almalinux | centos | fedora)
    .ci/package/rpm/build.sh
    ;;
  esac
}

case "$(uname -s)" in
Linux)
  build_linux
  ;;
MINGW*)
  .ci/package/win/build.sh
  ;;
esac
