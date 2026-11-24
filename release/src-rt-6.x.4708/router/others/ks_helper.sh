#!/bin/sh
#
# FreshTomato: Kill Switch FQDN helper (type 3)
# Copyright by pedro 2025 - 2026
# https://freshtomato.org/
#
# Revised: serialize instances, verify rules before clearing pending flags, and avoid ICMP-based gating of DNS lookups
#
export PATH=/bin:/usr/bin:/sbin:/usr/sbin:/home/root

. nvram_ops
. simple_lock_unlock.sh

PID=$$
MODE=$1
FILE=/tmp/kill-switch/fqdns
PIDFILE=/var/run/ks_helper.pid
SESSION_LOCK=/var/run/ks_helper.instance
FW_GUARD=/var/run/ks-firewall.guard
TAG="KS-helper[$PID]"
ATTEMPT=0
HAVE_FW_LOCK=0

LOGI() { logger -t "$TAG" "$*"; }
LOGE() { logger -p ERROR -t "$TAG" "$*"; }
LOGD() { logger -p DEBUG -t "$TAG" "$*"; }
LOGR() {
	if [ "$MODE" = update ] || [ "$ATTEMPT" -eq 1 ] || [ $((ATTEMPT % 10)) -eq 0 ]; then
		LOGE "$*"
	fi
}

case "$MODE" in
	add|update) ;;
	*) echo "usage: $0 update | add" >&2; exit 2 ;;
esac

cleanup() {
	trap - 0 HUP INT TERM
	[ "$HAVE_FW_LOCK" -eq 0 ] || simple_unlock
	flock -u 9 >/dev/null 2>&1 || :
	rm -f "$PIDFILE"
}

# Legacy /var/lock/firewall.lock is still shared with other FreshTomato scripts
# Unlike simple_lock(), this must NOT claim success if its timeout expires
acquire_legacy_lock() {
	local n=25
	while ! rm -- /var/lock/firewall.lock 2>/dev/null; do
		n=$((n - 1))
		[ "$n" -gt 0 ] || return 1
		sleep 1
	done
	HAVE_FW_LOCK=1
	return 0
}

has_pending() {
	[ -r "$FILE" ] || return 1
	awk '
		$1 !~ /^#/ && NF == 4 && $4 == "1" { found=1; exit }
		END { exit(found ? 0 : 1) }
	' "$FILE"
}

ipv6_enabled() {
	case "$(NG ipv6_service 2>/dev/null)" in
		native|native-pd|6to4|sit|other|6rd|6rd-pd) return 0 ;;
		*) return 1 ;;
	esac
}

# Parse only answer addresses, never the DNS server address
answer_ips() {
	awk '
		$1 == "Name:" { answer=1; next }
		answer && ($1 == "Address:" || ($1 == "Address" && $2 ~ /^[0-9]+:$/)) {
			ip=$1 == "Address:" ? $2 : $3
			sub(/#.*/, "", ip)
			if (ip ~ /^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$/ || (index(ip, ":") && ip ~ /^[0-9A-Fa-f:.]+$/))
				if (!seen[ip]++) print ip
		}
	'
}

# Check or insert each rule while holding both firewall locks
ensure_rule() {
	local tool=$1 ip=$2 reject=$3
	shift 3
	"$tool" -C FORWARD -d "$ip" "$@" -j REJECT --reject-with "$reject" >/dev/null 2>&1 && return 0
	"$tool" -I FORWARD -d "$ip" "$@" -j REJECT --reject-with "$reject" >/dev/null 2>&1 || return 1
	ADDED=1
}

# Rewrite by exact field match, not by interpolating an FQDN into a sed regex
# Caller must hold the FreshTomato firewall lock
mark_resolved() {
	local domain=$1 vpn=$2 wan=$3 temp
	temp=$(mktemp "$FILE.XXXXXX") || return 1
	# Preserve the source file's owner/mode (mktemp defaults to 0600)
	if ! cp -p "$FILE" "$temp"; then
		rm -f "$temp"
		return 1
	fi
	if ! awk -v d="$domain" -v v="$vpn" -v w="$wan" '
		NF == 4 && $1 == d && $2 == v && $3 == w && $4 == "1" {
			print $1, $2, $3, "0"
			next
		}
		{ print }
	' "$FILE" > "$temp"; then
		rm -f "$temp"
		return 1
	fi
	if ! mv -f "$temp" "$FILE"; then
		rm -f "$temp"
		return 1
	fi
}

process_domains() {
	local line domain vpn wan flag result ips ip tool reject n ok any status
	local rc had_error=0 generation current_generation stale=0

	[ -r "$FILE" ] || {
		LOGE "FQDN input file unavailable: $FILE"
		return 1
	}
	# Read the generation of the snapshot; do not apply DNS responses to a
	# different firewall generation when a restart happened during nslookup
	IFS= read -r generation < "$FILE" || return 1
	while IFS= read -r line || [ -n "$line" ]; do
		case "$line" in ""|\#*) continue ;; esac
		# Field splitting is intentional; glob expansion is disabled below
		set -f
		set -- $line
		set +f
		if [ "$#" -ne 4 ]; then
			LOGR "malformed FQDN entry (expected 4 fields)"
			had_error=1
			continue
		fi
		domain=$1 vpn=$2 wan=$3 flag=$4
		case "$flag" in
			0) [ "$MODE" = add ] && continue ;;
			1) ;;
			*) LOGR "invalid state flag for $domain: $flag"; had_error=1; continue ;;
		esac
		case "$domain" in
			""|-*|*[!a-zA-Z0-9._-]*)
				LOGR "invalid domain in FQDN file: $domain"
				had_error=1
				continue ;;
		esac
		case "$vpn:$wan" in
			*[!a-zA-Z0-9_.:+-]*|:*|*:)
				LOGR "invalid interface name for $domain"
				had_error=1
				continue ;;
		esac

		result= ips= rc=1
		n=1
		while [ "$n" -le 3 ]; do
			result=$(nslookup "$domain" 2>&1)
			rc=$?
			if [ "$rc" -eq 0 ]; then
				ips=$(printf '%s\n' "$result" | answer_ips)
				[ -n "$ips" ] && break
			fi
			n=$((n + 1))
			[ "$n" -le 3 ] && usleep 200000
		done

		if [ -z "$ips" ]; then
			LOGR "type: 3 - no usable DNS answers for '$domain' after 3 attempts (nslookup status $rc): $result"
			had_error=1
			continue
		fi

		ok=1 any=0 ADDED=0
		# Mutual exclusion with start_firewall() using the *same kernel lock*.
		# The shell does not hold this lock during a slow DNS lookup
		if ! flock 9; then
			LOGR "cannot acquire firewall generation lock"
			had_error=1
			continue
		fi

		# Reject stale DNS results after a firewall generation change
		current_generation=
		IFS= read -r current_generation < "$FILE" || :
		if [ "$generation" != "$current_generation" ]; then
			flock -u 9
			LOGD "FQDN snapshot changed during resolution; restarting scan"
			stale=1
			break
		fi

		if ! acquire_legacy_lock; then
			LOGR "cannot obtain FreshTomato firewall lock for $domain"
			flock -u 9
			had_error=1
			continue
		fi

		for ip in $ips; do
			case "$ip" in
				*:*) ipv6_enabled || continue; tool=ip6tables; reject=icmp6-port-unreachable ;;
				*) tool=iptables; reject=icmp-port-unreachable ;;
			esac
			any=1
			status=0
			ensure_rule "$tool" "$ip" "$reject" ! -o "$vpn" || status=1
			ensure_rule "$tool" "$ip" "$reject" -o "$wan" || status=1
			if [ "$status" -ne 0 ]; then
				LOGR "type: 3 - failed to install $tool rules for '$domain' ($ip)"
				ok=0
			fi
		done

		# Do not clear pending=1 if DNS returned only disabled IP families
		if [ "$any" -eq 0 ]; then
			ok=0
		fi

		if [ "$ok" -eq 1 ] && [ "$flag" = 1 ]; then
			if ! mark_resolved "$domain" "$vpn" "$wan"; then
				LOGR "unable to clear pending flag for $domain"
				ok=0
			fi
		fi
		simple_unlock
		HAVE_FW_LOCK=0
		flock -u 9

		if [ "$ok" -ne 1 ]; then
			had_error=1
			continue
		fi
		[ "$ADDED" -eq 1 ] && LOGI "type: 3 - added '$domain' [$wan]"
	done < "$FILE"

	[ "$stale" -eq 0 ] && [ "$had_error" -eq 0 ]
}


###################################################


if [ "$MODE" = add ]; then
	if [ ! -r "$FILE" ]; then
		LOGE "FQDN input file unavailable: $FILE"
		exit 1
	fi
	has_pending || exit 0
fi

# Session lock: one active helper. Cron update skips a busy instance;
# add waits up to 60 seconds for the instance to finish
exec 8>"$SESSION_LOCK" || { LOGE "cannot open session lock"; exit 1; }
if [ "$MODE" = update ]; then
	flock -n 8 || exit 0
else
	n=0
	until flock -n 8; do
		[ "$n" -lt 60 ] || { LOGE "helper instance busy for >60 seconds"; exit 1; }
		n=$((n + 1))
		sleep 1
	done
fi
printf '%s\n' "$PID" > "$PIDFILE" || exit 1

exec 9>"$FW_GUARD" || { LOGE "cannot open firewall guard"; exit 1; }
trap 'cleanup' 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

if [ "$MODE" = add ]; then
	has_pending || exit 0
	LOGI "trying to resolve pending FQDNs for FW ..."
else
	LOGD "perform scheduled FQDN checks ..."
fi

if [ "$MODE" = update ]; then
	ATTEMPT=1
	process_domains
	exit $?
fi

DELAY=15
while :; do
	ATTEMPT=$((ATTEMPT + 1))
	process_domains || :
	[ -r "$FILE" ] || { LOGE "FQDN input file disappeared: $FILE"; exit 1; }
	has_pending || break
	sleep "$DELAY"
	[ "$DELAY" -ge 60 ] || DELAY=$((DELAY * 2))
done
exit 0
