#!/bin/sh
# Split and unpack a NAND dump from the RTL9607C board.
#
#   ./nand-extract.sh <nand-full.bin | single-partition.bin> [outdir]
#
# Given a whole-chip 128MB image it carves the eight partitions, then walks each
# one: UBI images are parsed into their volumes, squashfs volumes are unpacked,
# and anything else is identified and left alone. Given a single partition dump
# (ubi_device.bin, userconfig.bin, ...) it skips straight to the walk.
#
# Everything here is read-only with respect to the board. This only ever reads
# files the board already gave us.
#
# Why a tool rather than doing it by hand each time: the partition offsets are
# easy to mistype, and a wrong offset does not error -- it yields a plausible
# image full of garbage, which is far more expensive than a crash.
set -e

IMG="$1"
OUT="${2:-$(dirname "$IMG")/extracted}"

[ -n "$IMG" ] && [ -f "$IMG" ] || { echo "usage: $0 <dump.bin> [outdir]" >&2; exit 1; }

SIZE=$(stat -c %s "$IMG")
mkdir -p "$OUT"

echo "input : $IMG ($SIZE bytes)"
echo "output: $OUT"
echo

# --- carve partitions, if this is a whole-chip image --------------------------
#
# Layout from the U-Boot env, confirmed two ways: the mtdparts string and the
# fl_*_bs offset variables agree, and the sizes total exactly 128MB.
if [ "$SIZE" -eq 134217728 ]; then
	echo "whole-chip image; carving 8 partitions"
	# name:offset:size  (decimal bytes, so no shell hex arithmetic surprises)
	for spec in \
		boot:0:1048576 \
		env:1048576:1048576 \
		ubi_device:2097152:53477376 \
		ubi_device_1:55574528:53477376 \
		userconfig:109051904:8388608 \
		tp_data:117440512:8388608 \
		paniclog:125829120:4194304 \
		defaults:130023424:4194304
	do
		name=${spec%%:*}; rest=${spec#*:}
		off=${rest%%:*}; len=${rest#*:}
		dd if="$IMG" of="$OUT/$name.bin" bs=4096 skip=$((off / 4096)) \
		   count=$((len / 4096)) status=none
		printf '  %-14s %9d bytes @ 0x%08x\n' "$name" "$len" "$off"
	done
	PARTS=$(ls "$OUT"/*.bin)
else
	echo "single-partition image"
	PARTS="$IMG"
fi
echo

# --- identify and unpack each partition ---------------------------------------
for p in $PARTS; do
	base=$(basename "$p" .bin)
	magic=$(dd if="$p" bs=4 count=1 status=none | od -An -tx1 | tr -d ' \n')
	printf '%-14s magic=%s ' "$base" "$magic"

	case "$magic" in
	55424923)  # "UBI#"
		echo "-> UBI"
		python3 "$(dirname "$0")/ubi-unpack.py" "$p" "$OUT/$base.ubi" || true
		# The interesting payload is usually a squashfs INSIDE a volume, not the
		# partition itself, so recurse rather than leaving the user a pile of
		# volN.bin to identify by hand.
		for v in "$OUT/$base.ubi"/vol*.bin; do
			[ -f "$v" ] || continue
			vm=$(dd if="$v" bs=4 count=1 status=none | od -An -tx1 | tr -d ' \n')
			case "$vm" in
			68737173|73717368)
				vb=$(basename "$v" .bin)
				if command -v unsquashfs >/dev/null 2>&1; then
					rm -rf "$OUT/$base.$vb.rootfs"
					if unsquashfs -q -d "$OUT/$base.$vb.rootfs" "$v" >/dev/null 2>&1; then
						echo "               $vb -> $base.$vb.rootfs ($(find "$OUT/$base.$vb.rootfs" -type f | wc -l) files)"
					fi
				fi
				;;
			esac
		done
		;;
	68737173|73717368)  # "hsqs" / "sqsh"
		echo "-> squashfs"
		if command -v unsquashfs >/dev/null 2>&1; then
			rm -rf "$OUT/$base.rootfs"
			unsquashfs -q -d "$OUT/$base.rootfs" "$p" >/dev/null 2>&1 \
				&& echo "               unpacked -> $base.rootfs ($(find "$OUT/$base.rootfs" -type f | wc -l) files)"
		else
			echo "               (no unsquashfs on PATH)"
		fi
		;;
	*)
		# Not a container we know. Say something useful about it anyway: a
		# partition that is entirely 0xff is erased, and saying so is more
		# informative than "unknown".
		if [ "$(tr -d '\377' < "$p" | wc -c)" -eq 0 ]; then
			echo "-> erased (all 0xff)"
		else
			echo "-> raw; $(strings "$p" 2>/dev/null | wc -l) printable strings"
		fi
		;;
	esac
done

echo
echo "done. tree:"
find "$OUT" -maxdepth 1 -mindepth 1 | sed 's|^|  |'
