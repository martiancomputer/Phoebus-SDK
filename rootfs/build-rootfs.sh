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

# --- s6 supervision suite: cross-build (static) + install the curated subset ---
# s6 is PID 1 (see rootfs/init); the full 148-binary suite is 89M static, so we
# only ship what milestone-1 supervision needs. Grow this list as services land.
HOST="${CROSS_COMPILE%-}" "$HERE/../s6/build.sh"
for b in s6-svscan s6-supervise s6-svstat s6-svc s6-svscanctl s6-svok; do
	cp "$HERE/../s6/staging/bin/$b" "$OUT/bin/$b"
done

# /init is the s6 stage-1 bootstrap, NOT the busybox init symlink
cp "$HERE/init" "$OUT/init"
chmod 0755 "$OUT/init" "$OUT/etc/s6/rc.boot" "$OUT/etc/s6/sv/getty-console/run"

echo "rootfs tree ready: $OUT"
echo "device-node spec for initramfs: $HERE/initramfs-devnodes.txt"
