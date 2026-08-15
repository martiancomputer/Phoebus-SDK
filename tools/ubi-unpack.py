#!/usr/bin/env python3
"""Split a raw UBI image into its volumes.

    ubi-unpack.py <ubi.bin> [outdir]

Reassembles each volume from its logical erase blocks and writes one file per
volume, then says what each one looks like.

Written because ubireader is not always installed and the format is simple
enough not to need it: every physical erase block carries a 64-byte EC header
("UBI#") giving the offsets of the VID header and the data area, and the VID
header ("UBI!") gives the volume id and the logical block number. Reassembly is
just sorting blocks by LEB number within each volume.

The one thing worth being careful about is that PEBs are NOT in order on flash
and LEB numbers are not contiguous across a volume's PEBs -- concatenating
blocks in physical order gives a corrupt image that still has a valid squashfs
magic at the front, which is exactly the kind of wrong answer that wastes hours.
"""
import os
import struct
import sys


def main():
    if len(sys.argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 1
    path = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else path + ".volumes"
    os.makedirs(out, exist_ok=True)

    data = open(path, "rb").read()
    if data[:4] != b"UBI#":
        print("  not a UBI image (no UBI# magic)", file=sys.stderr)
        return 1

    # EC header: vid_hdr_offset at 0x10, data_offset at 0x14, both big-endian.
    vid_off, data_off = struct.unpack(">II", data[16:24])

    # PEB size is not recorded in the header; find it by locating the next EC
    # header. Checking the plausible sizes in descending order of likelihood is
    # enough -- UBI PEBs are always a power-of-two NAND block size.
    peb = None
    for probe in (131072, 262144, 65536, 32768, 16384):
        if data[probe:probe + 4] == b"UBI#":
            peb = probe
            break
    if peb is None:
        print("  could not determine PEB size", file=sys.stderr)
        return 1

    print("  vid_hdr_offset=%d data_offset=%d peb=%d" % (vid_off, data_off, peb))

    vols = {}
    bad = 0
    for i in range(len(data) // peb):
        p = i * peb
        if data[p:p + 4] != b"UBI#":
            bad += 1
            continue
        vid = data[p + vid_off:p + vid_off + 64]
        if vid[:4] != b"UBI!":
            continue                      # erased or unmapped block
        vol_id, lnum = struct.unpack(">II", vid[8:16])
        vols.setdefault(vol_id, {})[lnum] = data[p + data_off:p + peb]

    if bad:
        print("  %d block(s) without an EC header (erased or bad)" % bad)

    for vol_id, lebs in sorted(vols.items()):
        # Sort by LEB number, not physical order -- see the module docstring.
        blob = b"".join(lebs[k] for k in sorted(lebs))
        # Internal layout volume; not user data.
        name = "vol%d" % vol_id if vol_id < 0x7FFFEF00 else "layout%d" % vol_id
        dest = os.path.join(out, name + ".bin")
        with open(dest, "wb") as f:
            f.write(blob)

        head = blob[:4]
        kind = {
            b"hsqs": "squashfs",
            b"sqsh": "squashfs",
            b"UBI#": "nested ubi",
            b"\x19\x85\x20\x03": "jffs2",
        }.get(head, "")
        if head[:4] == b"\x27\x05\x19\x56":
            kind = "u-boot uImage"
        print("  vol %-10s %3d LEBs  %9d bytes  %-14s %s"
              % (vol_id, len(lebs), len(blob), kind or "raw", dest))

    return 0


if __name__ == "__main__":
    sys.exit(main())
