#!/usr/bin/env python3
"""Validate only LED-related entries in an uncompressed newc initramfs.

Usage: check-image.py <initramfs_data.cpio> <staged-rootfs>
Never prints provisioning files or their contents.
"""
import hashlib
from pathlib import Path
import stat
import sys

archive, staging = Path(sys.argv[1]), Path(sys.argv[2])
data = archive.read_bytes()
entries = {}
offset = 0
while offset + 110 <= len(data):
    header = data[offset:offset+110]
    assert header[:6] in (b"070701", b"070702"), "not a newc archive"
    fields = [int(header[i:i+8], 16) for i in range(6, 110, 8)]
    mode, uid, gid, size, namesize = fields[1], fields[2], fields[3], fields[6], fields[11]
    start = offset + 110
    assert namesize > 0 and start + namesize <= len(data)
    name = data[start:start+namesize-1].decode("utf-8")
    assert data[start+namesize-1] == 0
    start = (start + namesize + 3) & ~3
    assert start + size <= len(data)
    if name == "TRAILER!!!":
        break
    if name.startswith("./"):
        name = name[2:]
    entries[name] = mode, uid, gid, data[start:start+size]
    offset = (start + size + 3) & ~3
else:
    raise AssertionError("missing archive trailer")

required = ["usr/sbin/gpio-daemon", "usr/sbin/gpioctl", "usr/sbin/gpio-lease-notify",
            "etc/phoebus/gpio.conf", "etc/s6/source/gpio-daemon/run",
            "etc/s6/source/gpio-daemon/type", "etc/s6/source/gpio-daemon/dependencies",
            "etc/s6/source/hostapd/finish", "etc/s6/source/hostapd-2g/finish",
            "usr/libexec/kmsg-relay", "usr/bin/sys-check", "usr/bin/sys-diag",
            "etc/s6/source/kmsg-relay/run"]
for name in required:
    mode, uid, gid, body = entries[name]
    assert uid == gid == 0, (name, "not root-owned")
    file = staging / name
    if stat.S_ISLNK(mode):
        # Kernel gen_init_cpio includes the terminating NUL in symlink data.
        assert name == "usr/sbin/gpio-lease-notify" and body in (b"gpioctl", b"gpioctl\0")
    else:
        assert not mode & 0o6022, (name, "setid or writable by non-owner")
        assert body == file.read_bytes(), (name, "stale embedded file")
        if name.endswith(("/run", "/finish", "/gpio-daemon", "/gpioctl", "/kmsg-relay", "/sys-check", "/sys-diag")):
            assert mode & 0o111 == 0o111

for name in ("usr/sbin/gpio-daemon", "usr/sbin/gpioctl"):
    body = entries[name][3]
    assert body[:6] == b"\x7fELF\x01\x02", "expected ELF32 big-endian"
    assert int.from_bytes(body[18:20], "big") == 8, "expected MIPS"
    assert len(body) <= 200*1024
    print(name, len(body), "bytes", hashlib.sha256(body).hexdigest())
for name, (_, _, _, body) in entries.items():
    assert not name.startswith("usr/sbin/phoebus-led"), "retired shell helper embedded"
    assert name not in ("usr/libexec/phoebus-kmsg-relay", "usr/bin/phoebus-check", "usr/bin/phoebus-diag"), "retired helper name embedded"
    assert not name.startswith("etc/s6/source/leds"), "retired service embedded"
    if name.startswith("etc/s6/source/") and name.endswith("/dependencies"):
        assert b"gpio-daemon" not in body.split() and b"leds" not in body.split(), "network LED dependency"
print("PASS: embedded MIPS binaries, current hooks/config, root ownership and independent service graph")
