#!/bin/sh

set -eux

bail() {
  echo >&2 "@"
  exit 1
}

header() {
  echo '################################################################################'
  echo "# $*"
  echo '################################################################################'
}

run_tests() {
  flavor="$1"
  shift

  header "Cleaning up leftovers from previous runs"

  for name in tinc tincd; do
    pkill -TERM -x "$name" || true
    pkill -KILL -x "$name" || true
  done

  if [ "$(id -u)" != 0 ]; then
    chown -R "${USER:-$(whoami)}" . || true
  fi

  mkdir -p sanitizer logs

  header "Running test flavor $flavor"

  ./.ci/build.sh "$flavor" "$@"

  if [ "${HOST:-}" = mingw ]; then
    echo >&2 "Integration tests cannot run under wine, skipping"
    return 0
  fi

  if [ -n "${HOST:-}" ]; then
    echo >&2 "Using higher test timeout for cross-compilation job $HOST"
    timeout=10
  else
    timeout=1
  fi

  code=0
  meson test -C "$flavor" --timeout-multiplier $timeout --verbose || code=$?

  tar -c -z -f "logs/tests.$flavor.tar.gz" "$flavor" sanitizer/ || true

  return $code
}

flavor=$1
shift

case "$flavor" in
default)
  run_tests default "$@"
  ;;
nolegacy)
  run_tests nolegacy -Dcrypto=nolegacy "$@"
  ;;
gcrypt)
  run_tests gcrypt -Dcrypto=gcrypt "$@"
  ;;
*)
  bail "unknown test flavor $1"
  ;;
esac
