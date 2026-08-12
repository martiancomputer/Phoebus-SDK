#!/bin/sh
# Receive NAND dumps uploaded from U-Boot. READ-ONLY on the board.
#
#   sudo ./tools/nand-dump.sh [iface]
#
# Brings the link up on 192.168.7.2 the same way tftp-serve.sh does, then runs
# tools/tftp-recv.py to catch `tftpput` uploads. Deliberately does NOT start
# tftpd.py: that one serves images for flashing and refuses writes, and keeping
# those two roles in separate processes means the flashing path can never be
# talked into accepting an upload.
#
# Everything the board does here is a read. `nand read` cannot modify flash.
# Do not use `ubi part`, `ubifsmount`, `nand write`, `nand erase`, `nand scrub`
# or `nand markbad` -- UBI attach in particular can rewrite erase-counter
# headers, which is a write to NAND.
set -e

NET=192.168.7
HOST=$NET.2
OUT=$(cd "$(dirname "$0")/.." && pwd)/nand-dumps

[ "$(id -u)" = 0 ] || { echo "run with sudo" >&2; exit 1; }

IF="$1"
if [ -z "$IF" ]; then
	for c in /sys/class/net/*; do
		i=$(basename "$c")
		case "$i" in lo|wl*|tailscale*|virbr*|docker*|Cloudflare*) continue ;; esac
		[ "$(cat "$c/carrier" 2>/dev/null)" = 1 ] && IF=$i && break
	done
fi
[ -n "$IF" ] || { echo "no wired interface with carrier; pass one explicitly" >&2; exit 1; }

mkdir -p "$OUT"
echo "interface : $IF"
echo "dump dir  : $OUT"

cleanup() {
	command -v nmcli >/dev/null 2>&1 && nmcli device set "$IF" managed yes 2>/dev/null
	return 0
}
trap cleanup EXIT INT TERM

if command -v nmcli >/dev/null 2>&1; then
	nmcli device set "$IF" managed no 2>/dev/null || true
	sleep 1
fi
ip link set "$IF" up
ip addr flush dev "$IF"
ip addr add "$HOST/24" dev "$IF"
sysctl -qw net.ipv4.conf."$IF".rp_filter=0 2>/dev/null || true
sysctl -qw net.ipv4.conf.all.rp_filter=0 2>/dev/null || true

printf 'waiting for link on %s ' "$IF"
i=0
while [ "$i" -lt 100 ]; do
	[ "$(cat "/sys/class/net/$IF/carrier" 2>/dev/null)" = 1 ] && break
	printf '.'; sleep 0.1; i=$((i + 1))
done
echo " $(cat "/sys/class/net/$IF/carrier" 2>/dev/null | sed 's/1/up/;s/0/NO CARRIER/')"

cat <<EOF

--- on the board (U-Boot), read-only -------------------------------------
setenv serverip $HOST
setenv ipaddr $NET.10

Layout, confirmed two ways -- the mtdparts string and the fl_*_bs offsets in
the U-Boot env agree exactly, and the eight sizes total 128MB:

  #  partition      offset      size        done?
  0  boot           0x0000000   0x0100000   yes (in boot_env.bin)
  1  env            0x0100000   0x0100000   yes (in boot_env.bin)
  2  ubi_device     0x0200000   0x3300000   yes (ubi_device.bin)
  3  ubi_device_1   0x3500000   0x3300000   --
  4  userconfig     0x6800000   0x0800000   --
  5  tp_data        0x7000000   0x0800000   --
  6  paniclog       0x7800000   0x0400000   --
  7  defaults       0x7C00000   0x0400000   --

Smallest and most useful first, so an interrupted session still gains ground.
userconfig is the one to get: stock's runtime switch/VLAN config should live
there, and that is the open question on the WAN port.

  # partition 4, userconfig -- 8MB
  nand read 0x84000000 0x6800000 0x800000
  tftpput 0x84000000 0x800000 $HOST:userconfig.bin

  # partition 7, defaults -- 4MB, factory defaults
  nand read 0x84000000 0x7C00000 0x400000
  tftpput 0x84000000 0x400000 $HOST:defaults.bin

  # partition 5, tp_data -- 8MB, calibration / MACs
  nand read 0x84000000 0x7000000 0x800000
  tftpput 0x84000000 0x800000 $HOST:tp_data.bin

  # partition 6, paniclog -- 4MB
  nand read 0x84000000 0x7800000 0x400000
  tftpput 0x84000000 0x400000 $HOST:paniclog.bin

  # partition 3, ubi_device_1 -- 51MB, the B-side firmware. Slowest; last.
  nand read 0x84000000 0x3500000 0x3300000
  tftpput 0x84000000 0x3300000 $HOST:ubi_device_1.bin

Already captured, no need to repeat:
  # partition 2, ubi_device -- stock rootfs, 51MB
  nand read 0x84000000 0x200000 0x3300000
  tftpput 0x84000000 0x3300000 $HOST:ubi_device.bin

  # partition 0+1, boot + env -- 2MB
  nand read 0x84000000 0x0 0x200000
  tftpput 0x84000000 0x200000 $HOST:boot_env.bin

DRAM is 256MB at 0x80000000 and U-Boot relocates to 0x8ff70000, so 0x84000000
has room for the 51MB read without touching it.

Note the env's own bootargs currently reads "lan=eth0.7" -- that is OUR leftover
from a manual setenv, persisted by the saveenv inside tryanother. It is not a
factory value and must not be read back as evidence about this board.
--------------------------------------------------------------------------

EOF

exec python3 "$(dirname "$0")/tftp-recv.py" "$OUT" "$HOST" 69
