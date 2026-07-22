#!/bin/sh
# Fetch the cross toolchain used by all Phoebus BSPs.
# Big-endian MIPS32r2 (interAptiv), glibc, GCC 14 — from Bootlin.
# The binary toolchain is intentionally NOT committed; this script pulls it.
set -e

TC_NAME="mips32--glibc--stable-2025.08-1"
TC_URL="https://toolchains.bootlin.com/downloads/releases/toolchains/mips32/tarballs/${TC_NAME}.tar.xz"
DEST="${1:-$(cd "$(dirname "$0")/.." && pwd)/toolchain}"

mkdir -p "$DEST"
if [ -d "$DEST/$TC_NAME/bin" ]; then
	echo "Toolchain already present: $DEST/$TC_NAME"
	exit 0
fi

echo "Downloading $TC_NAME ..."
curl -fL --retry 3 -o "$DEST/$TC_NAME.tar.xz" "$TC_URL"
echo "Extracting ..."
tar -C "$DEST" -xf "$DEST/$TC_NAME.tar.xz"
rm -f "$DEST/$TC_NAME.tar.xz"

echo
echo "Toolchain ready: $DEST/$TC_NAME/bin"
echo "  CROSS_COMPILE=mips-buildroot-linux-gnu-"
echo "  add to PATH: $DEST/$TC_NAME/bin"
