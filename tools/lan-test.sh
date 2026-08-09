#!/bin/sh
# End-to-end LAN test against the board: static IP, ping, ARP, throughput hint.
#
#   sudo ./tools/lan-test.sh [iface]
#
# Run it, plug into a LAN jack when prompted, wait. Everything is logged to
# /tmp/lan-test.log so the result survives having no internet while the cable
# is out of the upstream router.
#
# The board is 192.168.1.1/24 on br0 and runs no DHCP server, so we address
# ourselves statically. 192.168.1.0/24 is the board's LAN -- unlike the
# 192.168.7.0/24 used for TFTP, which exists only in U-Boot.
set -e

IF="${1:-enp3s0f3u1}"
BOARD=192.168.1.1
SELF=192.168.1.10
LOG=/tmp/lan-test.log

[ "$(id -u)" = 0 ] || { echo "run with sudo" >&2; exit 1; }
[ -e "/sys/class/net/$IF" ] || { echo "no such interface: $IF" >&2; exit 1; }

exec > >(tee "$LOG") 2>&1

cleanup() {
	command -v nmcli >/dev/null 2>&1 && nmcli device set "$IF" managed yes 2>/dev/null
	echo
	echo "interface handed back to NetworkManager -- replug upstream for internet"
	return 0
}
trap cleanup EXIT INT TERM

if command -v nmcli >/dev/null 2>&1; then
	nmcli device set "$IF" managed no 2>/dev/null || true
	sleep 1
fi
ip link set "$IF" up
ip addr flush dev "$IF"
ip addr add "$SELF/24" dev "$IF"

echo "== plug the cable into a LAN jack now =="
printf 'waiting for link '
i=0
while [ "$i" -lt 300 ]; do          # 30s
	[ "$(cat "/sys/class/net/$IF/carrier" 2>/dev/null)" = 1 ] && break
	printf '.'; sleep 0.1; i=$((i + 1))
done
echo
[ "$(cat "/sys/class/net/$IF/carrier" 2>/dev/null)" = 1 ] || {
	echo "NO LINK after 30s -- wrong jack, or the PHY is still down"; exit 1; }
echo "link up: $(cat /sys/class/net/$IF/speed) Mb/s $(cat /sys/class/net/$IF/duplex) duplex"
sleep 2                              # let the bridge learn

echo
echo "== ARP (layer 2) =="
arping -I "$IF" -c 4 "$BOARD" || echo "  ARP FAILED -- switch is not forwarding to the CPU"

echo
echo "== ICMP (layer 3) =="
ping -I "$IF" -c 10 -i 0.2 -W 2 "$BOARD" || echo "  PING FAILED"

echo
echo "== SSH port reachable? =="
# Try each probe by name. The first version of this hardcoded `nc`, which is
# not installed here under that name -- Arch ships nmap's `ncat` -- so the
# && chain fell through and reported the port CLOSED when dropbear was in
# fact listening on 0.0.0.0:22. Never let "tool missing" render as "port shut".
SSH_PROBE=""
for c in nc ncat netcat; do
	command -v "$c" >/dev/null 2>&1 && SSH_PROBE=$c && break
done
if [ -z "$SSH_PROBE" ]; then
	echo "  SKIPPED -- no nc/ncat/netcat on this host (says nothing about the port)"
elif "$SSH_PROBE" -z -w3 "$BOARD" 22 2>/dev/null; then
	echo "  port 22 OPEN (probed with $SSH_PROBE)"
else
	echo "  port 22 unreachable (probed with $SSH_PROBE)"
fi

echo
echo "== neighbour table =="
ip neigh show dev "$IF"
