#!/bin/sh
# Cross-build the shared router monitor; downloaded sources stay in .work.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT=${1:?usage: build-btop.sh ROOTFS_TREE}
OUT=$(CDPATH= cd -- "$OUT" && pwd)
CROSS_COMPILE=${CROSS_COMPILE:-mips-buildroot-linux-gnu-}
CXX=${CROSS_COMPILE}g++
READELF=${CROSS_COMPILE}readelf
STRIP=${CROSS_COMPILE}strip
JOBS=${JOBS:-2}
LINKAGE=${BTOP_LINKAGE:-auto}
VERSION=1.4.7
SHA256=933de2e4d1b2211a638be463eb6e8616891bfba73aef5d38060bd8319baeefc6
WORK="$HERE/.work/btop-$VERSION"
ARCHIVE="$HERE/.work/btop-$VERSION.tar.gz"

case "$LINKAGE" in auto|dynamic|static) ;; *) echo 'Invalid BTOP_LINKAGE: use auto, dynamic or static' >&2; exit 2 ;; esac
for tool in "$CXX" "$READELF" "$STRIP" curl tar sha256sum make gzip localedef; do
    command -v "$tool" >/dev/null || { echo "Missing build tool: $tool" >&2; exit 1; }
done
SYSROOT=$("$CXX" -print-sysroot)
mkdir -p "$HERE/.work"
if ! printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum -c - >/dev/null 2>&1; then
    curl -fL --retry 3 -o "$ARCHIVE" "https://github.com/aristocratos/btop/archive/refs/tags/v$VERSION.tar.gz"
    printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum -c -
fi
if [ ! -f "$WORK/Makefile" ]; then
    mkdir -p "$WORK"
    tar xf "$ARCHIVE" -C "$WORK" --strip-components=1
fi

# Keep link modes separate; invalidate objects if the cross-compiler changes.
compiler_id=$("$CXX" -dumpmachine):$("$CXX" -dumpfullversion)
if [ "$(cat "$WORK/.phoebus-compiler" 2>/dev/null || true)" != "$compiler_id" ]; then
    rm -rf -- "$WORK/obj-false" "$WORK/obj-true"
    printf '%s\n' "$compiler_id" > "$WORK/.phoebus-compiler"
fi
build_monitor() {
    mode=$1
    # The release archive is not a checkout: don't report the parent SDK's
    # unrelated Git revision as btop's upstream source revision.
    rm -f -- "$WORK/obj-$mode/config.h"
    make -C "$WORK" CXX="$CXX" PLATFORM=Linux ARCH=mips \
        THREADS="$JOBS" GPU_SUPPORT=false INTEL_GPU_SUPPORT=false \
        STATIC="$mode" STRIP=true QUIET=true \
        BUILDDIR="obj-$mode" TARGETDIR="bin-$mode" \
        ADDFLAGS='-EB -mips32r2 -mabi=32' \
        GIT_COMMIT= LDFLAGS=-latomic
}
static=false
[ "$LINKAGE" != static ] || static=true
if ! build_monitor "$static"; then
    [ "$LINKAGE" = auto ] || exit 1
    echo 'btop: dynamic build failed; trying a fully static build' >&2
    static=true
    build_monitor "$static"
fi

mkdir -p "$OUT/usr/libexec" "$OUT/usr/share/btop/themes" "$OUT/lib"
install -m 0755 "$WORK/bin-$static/btop" "$OUT/usr/libexec/btop"
cp -R "$WORK/themes/." "$OUT/usr/share/btop/themes/"
install -m 0644 "$WORK/LICENSE" "$OUT/usr/share/btop/LICENSE"

# Stage DT_NEEDED dependencies recursively from this same cross-toolchain,
# dereferencing SONAME symlinks. Never satisfy them with host libraries.
stage_dependencies() (
    elf=$1
    for lib in $("$READELF" -d "$elf" | sed -n 's/.*Shared library: \[\([^]]*\)\].*/\1/p'); do
        [ ! -e "$OUT/lib/$lib" ] || continue
        source=$("$CXX" "-print-file-name=$lib")
        for dir in "$SYSROOT/lib" "$SYSROOT/usr/lib"; do
            [ ! -f "$dir/$lib" ] || source="$dir/$lib"
        done
        [ -f "$source" ] || { echo "btop: missing target library $lib" >&2; exit 1; }
        cp -L "$source" "$OUT/lib/$lib"
        chmod 0644 "$OUT/lib/$lib"
        "$STRIP" "$OUT/lib/$lib"
        stage_dependencies "$OUT/lib/$lib"
    done
)
stage_dependencies "$OUT/usr/libexec/btop"
interpreter=$("$READELF" -l "$OUT/usr/libexec/btop" | sed -n 's/.*Requesting program interpreter: \([^]]*\)].*/\1/p')
if [ -n "$interpreter" ]; then
    [ "$interpreter" = /lib/ld.so.1 ] || { echo "Unexpected MIPS interpreter: $interpreter" >&2; exit 1; }
    cp -L "$SYSROOT/lib/ld.so.1" "$OUT/lib/ld.so.1"
    "$STRIP" "$OUT/lib/ld.so.1"
fi

# btop's Unicode UI needs real LC_CTYPE data, not just LANG=... or --force-utf.
# Use the target glibc's locale source and big-endian output (no host archive).
mkdir -p "$OUT/usr/lib/locale/C.utf8"
gzip -dc "$SYSROOT/usr/share/i18n/charmaps/UTF-8.gz" > "$WORK/UTF-8"
I18NPATH="$SYSROOT/usr/share/i18n" localedef --no-archive --big-endian \
    -i "$SYSROOT/usr/share/i18n/locales/C" \
    -f "$WORK/UTF-8" "$OUT/usr/lib/locale/C.utf8"
echo "btop: $VERSION installed (static=$static, MIPS32r2 big-endian)"
