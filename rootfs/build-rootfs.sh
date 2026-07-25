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
mkdir -p "$OUT"/proc "$OUT"/sys "$OUT"/dev "$OUT"/tmp "$OUT"/root "$OUT"/mnt \
         "$OUT"/run "$OUT"/var/run "$OUT"/var/log

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

# --- wireless_tools: iwconfig/iwlist/iwpriv for the RTL8832BR (WEXT ioctls, no
# libnl needed since the vendor driver selects WIRELESS_EXT). Source lives in the
# vendor SDK; needs -lm for iwcommon's log10/ceil.
if [ -d "$WT_SRC" ]; then
	make -C "$WT_SRC" clean >/dev/null 2>&1 || true
	make -C "$WT_SRC" CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar \
		RANLIB=${CROSS_COMPILE}ranlib LDFLAGS="-lm" iwconfig iwlist iwpriv iwgetid
	for b in iwconfig iwlist iwpriv iwgetid; do
		cp "$WT_SRC/$b" "$OUT/sbin/$b"; ${CROSS_COMPILE}strip "$OUT/sbin/$b" || true
	done
	cp "$SYSROOT/lib/libm.so.6" "$OUT/lib/" 2>/dev/null || true
fi

# shared glibc runtime: the s6 binaries' interpreter (/lib/ld.so.1) + libc
SYSROOT="$(${CROSS_COMPILE}gcc -print-sysroot)"
mkdir -p "$OUT/lib"
cp "$SYSROOT/lib/ld.so.1" "$SYSROOT/lib/libc.so.6" "$OUT/lib/"
${CROSS_COMPILE}strip "$OUT/lib/ld.so.1" "$OUT/lib/libc.so.6" 2>/dev/null || true

# /init is the s6 stage-1 bootstrap, NOT the busybox init symlink
cp "$HERE/init" "$OUT/init"
chmod 0755 "$OUT/init" "$OUT/etc/s6/rc.boot" "$OUT/etc/s6/sv/getty-console/run"

echo "rootfs tree ready: $OUT"
echo "device-node spec for initramfs: $HERE/initramfs-devnodes.txt"
