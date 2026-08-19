#!/bin/sh
# Build a minimal static busybox rootfs tree for Phoebus boards.
# Produces $OUT (default ./rootfs-tree) ready to be turned into an
# initramfs or squashfs by the consuming BSP's build.sh.
#
# Requires: cross toolchain on PATH (see ../scripts/fetch-toolchain.sh),
#           CROSS_COMPILE (default mips-buildroot-linux-gnu-).
set -e

HERE=$(cd "$(dirname "$0")" && pwd)
CROSS_COMPILE="${CROSS_COMPILE:-mips-buildroot-linux-gnu-}"
BB_VER="${BB_VER:-1.37.0}"
OUT="${1:-$HERE/rootfs-tree}"
WORK="$HERE/.work"

mkdir -p "$WORK"
cd "$WORK"
if [ ! -d "busybox-$BB_VER" ]; then
	curl -fL --retry 3 -O "https://busybox.net/downloads/busybox-$BB_VER.tar.bz2"
	tar xf "busybox-$BB_VER.tar.bz2"
fi

cp "$HERE/busybox-$BB_VER.config" "busybox-$BB_VER/.config"
make -C "busybox-$BB_VER" ARCH=mips CROSS_COMPILE="$CROSS_COMPILE" oldconfig
make -C "busybox-$BB_VER" ARCH=mips CROSS_COMPILE="$CROSS_COMPILE" -j"$(nproc)"
rm -rf "$OUT"
make -C "busybox-$BB_VER" ARCH=mips CROSS_COMPILE="$CROSS_COMPILE" CONFIG_PREFIX="$OUT" install

# overlay the tracked /etc skeleton (fstab/passwd/group + s6 service tree)
cp -a "$HERE/etc" "$OUT/"

# ...and the tracked /usr skeleton. This is not optional decoration: it carries
# usr/share/udhcpc/default.script, the callback busybox udhcpc execs to apply a
# lease. Without it the WAN negotiates a lease and then configures nothing --
# no address, no route, no resolv.conf -- which looks like a dead WAN rather
# than a missing file.
#
# It was omitted here for a long time and went unnoticed because the staging
# tree already had a copy from an earlier manual build, so incremental rebuilds
# kept working while a clean clone would have produced a broken image.
#
# busybox install has already populated $OUT/usr/bin with applet symlinks, so
# copy the CONTENTS over the top rather than the directory itself; cp -a of the
# directory would nest it as usr/usr on a second run.
cp -a "$HERE/usr/." "$OUT/usr/"
mkdir -p "$OUT"/proc "$OUT"/sys "$OUT"/dev "$OUT"/tmp "$OUT"/root "$OUT"/mnt \
         "$OUT"/run "$OUT"/var/run "$OUT"/var/log "$OUT"/home/admin

# --- s6 supervision suite: cross-build (dynamic) + install ---
# Install the WHOLE built suite (bin + libexec). The binaries are ~68K dynamic
# each (~10M total), and s6-rc reaches a lot of them through exec chains at
# runtime (s6-svlisten, s6-ftrigrd, s6-sudod, s6-ipcserverd, s6-fdholderd, the
# /libexec helpers, ...). Hand-curating that set is fragile; shipping the lot is
# the robust choice for a RAM-booted board with plenty of memory.
HOST="${CROSS_COMPILE%-}" "$HERE/../s6/build.sh"
cp "$HERE/../s6/staging/bin/"* "$OUT/bin/"
mkdir -p "$OUT/libexec"
cp "$HERE/../s6/staging/libexec/"* "$OUT/libexec/"

# --- s6-hpd: our own hotplug daemon (the procd/udev role s6 lacks) ---
${CROSS_COMPILE}gcc -O2 -Wall -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64 \
	"$HERE/../s6-hpd/s6-hpd.c" -o "$OUT/bin/s6-hpd"
${CROSS_COMPILE}strip "$OUT/bin/s6-hpd" 2>/dev/null || true

SYSROOT="$(${CROSS_COMPILE}gcc -print-sysroot)"

# --- wireless_tools: iwconfig/iwlist/iwpriv for the RTL8832BR (WEXT ioctls, no
# libnl needed since the vendor driver selects WIRELESS_EXT). Source lives in the
# vendor SDK; needs -lm for iwcommon's log10/ceil.
# WT_SRC was never set, so this whole block silently did nothing on every build
# and the image shipped without iwpriv. That is not a missing convenience: iwpriv
# is the ONLY way to configure the Realtek wireless drivers. Stock's rtk_wlan.sh
# makes 54 iwpriv calls at interface bring-up (powerpercent, txbf, txbf_mu,
# deny_legacy, ofdma_enable, the whole calibration set); without the binary we
# make zero, and both radios run on whatever the vendor compiled in as defaults.
#
# Default to the AX10v3 GPL drop, which is this exact board's own source. Still
# overridable, and still outside the repo -- see the manifest at the end, which
# now reports iwpriv so a build without it is loud rather than silent.
: "${WT_SRC:=$HERE/../../OpenWRT1500/AX10v3_GPL/rtl8198/user/wireless_tools.29}"
if [ -d "$WT_SRC" ]; then
	make -C "$WT_SRC" clean >/dev/null 2>&1 || true
	make -C "$WT_SRC" CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar \
		RANLIB=${CROSS_COMPILE}ranlib LDFLAGS="-lm" iwconfig iwlist iwpriv iwgetid
	for b in iwconfig iwlist iwpriv iwgetid; do
		cp "$WT_SRC/$b" "$OUT/sbin/$b"; ${CROSS_COMPILE}strip "$OUT/sbin/$b" || true
	done
	# iwpriv links against libiw, which is built as a shared object here. Copying
	# only the binaries gives "libiw.so.29: cannot open shared object file" at
	# runtime -- and since nothing calls iwpriv during boot, that error would
	# only surface the first time someone tried to use it.
	cp "$WT_SRC"/libiw.so.* "$OUT/lib/" 2>/dev/null || true
	cp "$SYSROOT/lib/libm.so.6" "$OUT/lib/" 2>/dev/null || true
else
	echo "build-rootfs: WT_SRC not found ($WT_SRC) -- no iwpriv, radios stay unconfigured" >&2
fi

# shared glibc runtime: the s6 binaries' interpreter (/lib/ld.so.1) + libc
mkdir -p "$OUT/lib"
cp "$SYSROOT/lib/ld.so.1" "$SYSROOT/lib/libc.so.6" "$OUT/lib/"
${CROSS_COMPILE}strip "$OUT/lib/ld.so.1" "$OUT/lib/libc.so.6" 2>/dev/null || true

# /init is the s6 stage-1 bootstrap, NOT the busybox init symlink
cp "$HERE/init" "$OUT/init"
# NB: no etc/s6/rc.boot here. It was a leftover from an earlier layout -- the
# init flow is /init -> s6-rc-compile -> s6-svscan /run/service. chmod applies
# the mode to the operands that do exist but still exits 1 on the missing one,
# and under `set -e` that killed the caller's build.sh right after this step.
chmod 0755 "$OUT/init" "$OUT/etc/s6/sv/getty-console/run"

# busybox must be setuid root or applets that drop/raise privilege cannot work.
# Without this `su` fails outright ("su: must be suid to work properly"), which
# left the admin SSH account unable to do anything privileged -- restarting a
# service returned "s6-rc: fatal: unable to take locks: Permission denied".
# The initramfs is generated with CONFIG_INITRAMFS_ROOT_UID/GID=0, so the file
# lands root-owned in the image and the setuid bit is meaningful there even
# though it is built unprivileged here.
chmod u+s "$OUT/bin/busybox"

# --- inject real credentials (Wi-Fi passphrase + admin hash) if present ---
# No-op without secrets/phoebus.env: the tree keeps locked accounts + a
# placeholder Wi-Fi passphrase. See ../PROVISIONING.md.
"$HERE/provision-secrets.sh" "$OUT" || true

# --- report which third-party binaries made it in ---
#
# hostapd, dnsmasq, dropbear, iperf3 and tcpdump are cross-built from trees that
# do NOT live in this repo (see net/README.md). Nothing here builds or fetches
# them, so on a clean clone they are simply absent -- and every one of them
# fails in a way that looks like something else:
#
#   udhcpc default.script  no address after a good lease   -> "WAN is dead"
#   hostapd                no SSID on the air              -> "radio is broken"
#   dnsmasq                LAN clients resolve nothing     -> "no internet"
#   dropbear               no SSH, no way in but serial
#
# So say it out loud at build time rather than discovering it on the board.
# Deliberately a warning, not an error: a kernel-only or network-only image is
# a legitimate thing to build while bisecting.
echo
echo "third-party binaries in this rootfs:"
missing=0
for p in usr/share/udhcpc/default.script sbin/hostapd usr/sbin/dnsmasq \
         usr/sbin/dropbear usr/bin/iperf3 usr/sbin/tcpdump sbin/iptables \
         sbin/iwpriv sbin/tc; do
	if [ -e "$OUT/$p" ]; then
		printf '  present : %s\n' "$p"
	else
		printf '  MISSING : %s\n' "$p"
		missing=$((missing + 1))
	fi
done
[ "$missing" -gt 0 ] && echo "  ($missing missing -- see net/README.md; not built by this script)"

echo
echo "rootfs tree ready: $OUT"
echo "device-node spec for initramfs: $HERE/initramfs-devnodes.txt"
