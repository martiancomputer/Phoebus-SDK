#!/bin/sh
#
# Phoebus-OS one-time boot bring-up (run once by /init before s6-svscan).
# Network + per-unit MAC provisioning. Long-lived daemons belong in the s6
# scandir (/etc/s6/sv/*), NOT here.
# --------------------------------------------------------------------------

ip link set lo up 2>/dev/null

# --- per-unit MAC from kernel cmdline: ethaddr=AA:BB:CC:DD:EE:FF -------------
# (rtl86900 driver installs a 00:00:00:01:00:02 placeholder; real MAC normally
#  comes from flash apmib, which this rootfs cannot read yet.)
MAC=""
for a in $(cat /proc/cmdline 2>/dev/null); do
	case "$a" in
		ethaddr=*) MAC="${a#ethaddr=}" ;;
	esac
done
if [ -n "$MAC" ]; then
	for i in /sys/class/net/eth* /sys/class/net/nas*; do
		[ -e "$i" ] || continue
		ip link set dev "$(basename "$i")" address "$MAC" 2>/dev/null
	done
	echo "rc.boot: MAC provisioned $MAC"
fi

# --- bring links up + test address on eth0 ---
for i in /sys/class/net/*; do
	dev=$(basename "$i")
	[ "$dev" = "lo" ] && continue
	ip link set "$dev" up 2>/dev/null
done
ip addr add 192.168.1.1/24 dev eth0 2>/dev/null && echo "rc.boot: eth0 192.168.1.1/24"
