#!/bin/sh

set -eux

dir="${1:-build_muon}"

if muon version >/dev/null; then
  MUON=muon
elif muon-meson version >/dev/null; then
  MUON=muon-meson
else
  echo 'Muon not found' >&2
  exit 1
fi

if samu --version >/dev/null; then
  SAMU=samu
elif ninja --version >/dev/null; then
  SAMU=ninja
else
  echo 'Neither samu nor ninja found' >&2
  exit 1
fi

$MUON setup "$dir"
$SAMU -C "$dir"
"$dir"/src/tinc --version
