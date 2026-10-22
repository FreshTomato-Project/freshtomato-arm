#!/bin/sh
#
# Check manual page sources with groff warnings enabled. Under make check, this
# also covers the utility man pages outside docs.

set -u

: "${MAN:=man}"
: "${srcdir:=.}"

if test "$srcdir" = "."; then
	case "$0" in
	*/*)
		srcdir=${0%/*}
		;;
	esac
fi

if ! command -v "$MAN" >/dev/null 2>&1; then
	echo "SKIP: man command not found"
	exit 77
fi

if command -v locale >/dev/null 2>&1; then
	if ! LC_ALL=C.UTF-8 locale charmap >/dev/null 2>&1; then
		echo "SKIP: C.UTF-8 locale not available"
		exit 77
	fi
fi

if ! "$MAN" --help 2>&1 | grep -- "--warnings" >/dev/null 2>&1; then
	echo "SKIP: man command does not support --warnings"
	exit 77
fi

failed=0
found=0

if test "${MANPAGES+x}" = x; then
	explicit_pages=1
	set -- $MANPAGES
else
	explicit_pages=0
	set -- "$srcdir"/*.[0-9]
	if test "${top_srcdir+x}" = x; then
		set -- "$@" \
			"$top_srcdir"/utils/pscap.8 \
			"$top_srcdir"/utils/filecap.8 \
			"$top_srcdir"/utils/captest.8 \
			"$top_srcdir"/utils/cap-audit/cap-audit.8
	fi
	if test "${top_builddir+x}" = x; then
		set -- "$@" "$top_builddir"/utils/netcap.8
	fi
fi

for page
do
	if ! test -e "$page"; then
		case "$page" in
		*\**)
			test $explicit_pages -eq 1 || continue
			;;
		esac
		failed=1
		echo "$page: not found"
		continue
	fi
	found=1

	output=$(
		LC_ALL=C.UTF-8 MANROFFSEQ='' MANWIDTH=80 \
			"$MAN" --warnings -E UTF-8 -l -Tutf8 -Z "$page" \
			2>&1 >/dev/null
	)
	status=$?

	if test $status -ne 0 || test -n "$output"; then
		failed=1
		echo "$page:"
		if test -n "$output"; then
			printf '%s\n' "$output"
		fi
		if test $status -ne 0; then
			echo "man exited with status $status"
		fi
	fi
done

if test $found -eq 0; then
	echo "No manual pages found"
	exit 1
fi

exit $failed
