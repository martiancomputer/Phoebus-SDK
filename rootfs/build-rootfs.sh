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

# --- s6 supervision suite: cross-build (dynamic) + install curated set ---
# s6 links against a shared glibc runtime staged in /lib (below); binaries are
# ~68K each, so the whole supervision + s6-rc + execline toolkit is a few MB.
# Grow this list as services land.
HOST="${CROSS_COMPILE%-}" "$HERE/../s6/build.sh"
S6BIN="$HERE/../s6/staging/bin"
for b in \
	s6-svscan s6-supervise s6-svc s6-svok s6-svstat s6-svscanctl s6-svwait s6-svlisten1 s6-log \
	s6-rc s6-rc-init s6-rc-compile s6-rc-update s6-rc-db \
	s6-setuidgid s6-envuidgid s6-applyuidgid s6-envdir s6-notifyoncheck \
	execlineb exec if ifelse ifte foreground background forx forstdin importas define \
	multisubstitute redirfd fdmove fdclose heredoc wait cd umask elgetpositionals \
	emptyenv export unexport trap pipeline withstdinas exit loopwhilex ; do
	cp "$S6BIN/$b" "$OUT/bin/$b"
done

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
