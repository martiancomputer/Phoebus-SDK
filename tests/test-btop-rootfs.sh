#!/bin/sh
# Verify a generated rootfs, optionally executing MIPS binaries under QEMU.
set -eu
OUT=${1:?usage: test-btop-rootfs.sh ROOTFS_TREE [QEMU_MIPS]}
QEMU=${2:-}
READELF=${CROSS_COMPILE:-mips-buildroot-linux-gnu-}readelf
test -x "$OUT/usr/bin/btop"
test -x "$OUT/usr/libexec/btop"
test -r "$OUT/etc/btop.conf"
test -r "$OUT/usr/lib/locale/C.utf8/LC_CTYPE"
test -r "$OUT/usr/share/btop/LICENSE"
sh -n "$OUT/usr/bin/btop"
"$READELF" -h "$OUT/usr/libexec/btop" | grep -q 'Class:.*ELF32'
"$READELF" -h "$OUT/usr/libexec/btop" | grep -q 'Data:.*big endian'
"$READELF" -h "$OUT/usr/libexec/btop" | grep -q 'Machine:.*MIPS'
for dir in bin sbin usr/bin usr/sbin; do
    test ! -e "$OUT/$dir/top"
    test ! -L "$OUT/$dir/top"
done
for elf in "$OUT/usr/libexec/btop" "$OUT"/lib/*; do
    [ -f "$elf" ] || continue
    "$READELF" -h "$elf" >/dev/null 2>&1 || continue
    for lib in $("$READELF" -d "$elf" 2>/dev/null | sed -n 's/.*Shared library: \[\([^]]*\)\].*/\1/p'); do
        test -e "$OUT/lib/$lib" || test -e "$OUT/usr/lib/$lib" || {
            echo "Missing runtime dependency: $lib" >&2; exit 1;
        }
    done
done
if [ -n "$QEMU" ]; then
    "$QEMU" -L "$OUT" "$OUT/usr/libexec/btop" --version
    applets=$("$QEMU" -L "$OUT" "$OUT/bin/busybox" --list)
    if printf '%s\n' "$applets" | grep -qx top; then
        echo 'BusyBox still contains the top applet' >&2; exit 1
    fi
fi
echo 'PASS: MIPS32 big-endian btop, UTF-8 data, resolved runtime libraries, no BusyBox top'
