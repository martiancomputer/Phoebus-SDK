#!/bin/sh
# Package the 6.18 RTL9607C kernel the way the vendor Luna SDK does
# (see rtl8198d-sdk-main/Makefile.image "vmimg" + tools/mkrom_vmimg_luna.sh).
#
# Products in images/:
#   uImage        - 2k page-aligned U-Boot image (lzma kernel, load 0x80001000)
#   vm.img        - genhead-wrapped kernel [+ rootfs when images/rootfs exists]
#
# The vendor's signed-FIT variant (build_phoebus_fit.sh) needs the RSA key
# tools/PHOEBUS_UOOT_FIT_KEY which is NOT shipped in the GPL dumps.  If the
# device's U-Boot enforces FIT signature verification, a self-generated key
# will not be accepted; the plain uImage path below matches the non-secure
# vendor flow.
set -e

SDKDIR=$(cd "$(dirname "$0")/.." && pwd)
LINUXDIR=${LINUXDIR:-$SDKDIR/linux-6.18.x}
IMAGEDIR=$SDKDIR/images
GENHEAD=$SDKDIR/tools/packimg/genhead

KERNEL_KEY=0xa0000203
ROOTFS_KEY=0xa0000403
FLASH_BASE=0xbd000000

mkdir -p "$IMAGEDIR"

[ -f "$LINUXDIR/arch/mips/boot/uImage.lzma" ] || {
	echo "uImage.lzma missing - build it first:"
	echo "  make ARCH=mips CROSS_COMPILE=mips-buildroot-linux-gnu- uImage.lzma"
	exit 1
}

# 2k page alignment for NAND (vendor mk_img_2k_alignment)
sz=$(stat --printf="%s" "$LINUXDIR/arch/mips/boot/uImage.lzma")
pagecnt=$(( (sz + 2047) / 2048 ))
dd if="$LINUXDIR/arch/mips/boot/uImage.lzma" ibs=2k count=$pagecnt \
   of="$IMAGEDIR/uImage" conv=sync status=none
echo "images/uImage: $sz -> $(stat --printf=%s "$IMAGEDIR/uImage") bytes (2k aligned)"

# genhead-wrapped vm.img (kernel part; rootfs appended when present)
tmp=$IMAGEDIR/.tmp
mkdir -p "$tmp"
"$GENHEAD" -i "$IMAGEDIR/uImage" -o "$tmp/uImage.hdr" -k $KERNEL_KEY -f $FLASH_BASE
cat "$tmp/uImage.hdr" "$IMAGEDIR/uImage" > "$tmp/uImage_new"

if [ -f "$IMAGEDIR/rootfs" ]; then
	"$GENHEAD" -i "$IMAGEDIR/rootfs" -o "$tmp/rootfs.hdr" -k $ROOTFS_KEY -f $FLASH_BASE
	cat "$tmp/rootfs.hdr" "$IMAGEDIR/rootfs" > "$tmp/rootfs_new"
	cat "$tmp/uImage_new" "$tmp/rootfs_new" > "$IMAGEDIR/vm.img"
	echo "images/vm.img: kernel + rootfs"
else
	cat "$tmp/uImage_new" > "$IMAGEDIR/vm.img"
	echo "images/vm.img: kernel only (no images/rootfs yet)"
fi
rm -rf "$tmp"
ls -la "$IMAGEDIR"
