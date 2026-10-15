#!/bin/sh
# Exercise only the conntrack initialization block, never live networking.
set -eu
SDK=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BLOCK=$(awk '/^CONNTRACK_MAX=/{copy=1} copy{print} /^echo "network: Linux conntrack capacity /{exit}' \
	"$SDK/rootfs/etc/s6/scripts/network-up")
[ -n "$BLOCK" ]
TEST_DIR=$(mktemp -d)
EXPECTED='nf_conntrack_max=16384
nf_conntrack_tcp_timeout_established=600
nf_conntrack_tcp_timeout_syn_sent=30
nf_conntrack_tcp_timeout_syn_recv=30
nf_conntrack_tcp_timeout_fin_wait=30
nf_conntrack_tcp_timeout_close_wait=30
nf_conntrack_tcp_timeout_last_ack=30
nf_conntrack_tcp_timeout_time_wait=30
nf_conntrack_tcp_timeout_close=1
nf_conntrack_tcp_timeout_max_retrans=300
nf_conntrack_tcp_timeout_unacknowledged=300
nf_conntrack_udp_timeout=30
nf_conntrack_udp_timeout_stream=180
nf_conntrack_generic_timeout=600'
cleanup() {
	for setting in $EXPECTED; do
		rm -f "$TEST_DIR/${setting%%=*}"
	done
	rmdir "$TEST_DIR"
}
trap cleanup EXIT HUP INT TERM

# Replace only the proc path; the actual startup logic remains identical.
TEST_BLOCK=$(printf '%s\n' "$BLOCK" | sed "s|/proc/sys/net/netfilter/|$TEST_DIR/|g")
for setting in $EXPECTED; do
	printf '0\n' > "$TEST_DIR/${setting%%=*}"
done
sh -c "$TEST_BLOCK"
for setting in $EXPECTED; do
	[ "$(cat "$TEST_DIR/${setting%%=*}")" = "${setting#*=}" ]
done

# A rejected write must fail the network oneshot, not claim success.
FAIL_BLOCK=$(printf '%s\n' "$BLOCK" | sed 's|/proc/sys/net/netfilter/\$key|/dev/full|g')
if sh -c "$FAIL_BLOCK" 2>/dev/null; then
	echo 'FAIL: conntrack write error was ignored' >&2
	exit 1
fi

# A write that does not read back correctly must also fail.
if sh -c 'cat() { printf "4096\n"; }; eval "$1"' sh "$TEST_BLOCK" 2>/dev/null; then
	echo 'FAIL: conntrack readback mismatch was ignored' >&2
	exit 1
fi
if sh -c 'cat() { case "$1" in */nf_conntrack_tcp_timeout_established) printf "599\n";; *) command cat "$@";; esac; }; eval "$1"' \
	sh "$TEST_BLOCK" >/dev/null 2>&1; then
	echo 'FAIL: timeout readback mismatch was ignored' >&2
	exit 1
fi
echo 'PASS: conntrack capacity, stock timeouts and startup failure checks'
