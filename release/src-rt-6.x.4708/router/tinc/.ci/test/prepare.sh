#!/bin/sh

set -eux

if [ -n "${HOST:-}" ]; then
  update-binfmts --enable
  rm -f /dev/net/tun
fi

if [ -n "$CI" ]; then
  # Workaround for ip netns exec messing with /sys mount in containers
  mount -t sysfs --make-private sysfs $(mktemp -d)
fi
