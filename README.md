# Phoebus-SDK

Shared, **kernel-version-independent** support code for Realtek Phoebus
(RTL9607C / RTL9607Cv2 — big-endian MIPS32r2 interAptiv) board support packages.

Consumed as a git submodule by the per-kernel BSPs:

| Repo | Kernel line |
|------|-------------|
| [PhoebusBSP-6](https://github.com/martiancomputer/PhoebusBSP-6) | LTS (currently 6.18.y) |
| [PhoebusBSP-7](https://github.com/martiancomputer/PhoebusBSP-7) | mainline / bleeding edge |

Two kinds of thing live here, and the split is deliberate:

- **`vendor/`** — the **pristine** SoC truth as extracted from the vendor GPL
  drop, *not* adapted to any kernel. Each BSP carries its own patch series that
  ports this to its kernel version. One copy of the ~120 MB vendor SDK serves
  every kernel line, and each port stays a reviewable diff.
- **everything else** — the userspace and packaging that is genuinely
  version-independent: the s6 init system, the rootfs, the image tools.

## Layout

```
vendor/
  realtek-net/     Realtek drivers/net/ethernet/realtek overlay, as-shipped:
                   rtl86900 (switch + GPON/EPON + NIC + FleetConntrack), the other
                   selectable PHY/switch drivers (rtl82xx, rtk-ms-phyOnly, rtl8367, ...)
                   and rtk_* helpers. Upstream-owned files (r8169, 8139*, atp) excluded.
  platform/        MIPS BSP: arch/mips/rtl9607c, dts/realtek, mach-rtl960xc headers,
                   drivers/{clk/realtek,gpio/gpio-rtk-soc.c,watchdog/rtl819x_wdt.c}
  include/         vendor headers: net/rtl, soc/cortina, dt-bindings/soc/9607xc_irqs.h

rootfs/            the Phoebus-OS root filesystem
  build-rootfs.sh    builds the whole tree (busybox + s6 + s6-hpd + tools)
  provision-secrets.sh  stamps real credentials into a built tree (see PROVISIONING.md)
  init               s6 stage-1 (PID 1 bootstrap) — NOT busybox init
  etc/s6/            the service tree: source/ (s6-rc db), sv/ (scandir seed), scripts/
  busybox-1.37.0.config

s6/build.sh        cross-builds the s6 supervision suite (skalibs, execline, s6, s6-rc,
                   s6-linux-init) for BE MIPS
s6-hpd/s6-hpd.c    our hotplug daemon — the NETLINK_KOBJECT_UEVENT role s6 lacks
                   (the piece procd does well and s6 has no answer for)

ap/README.md       hostapd cross-build notes (nl80211 + libnl, OpenSSL for WPA3/SAE)
net/README.md      network userspace notes (dropbear/libxcrypt, iptables, iperf3, tcpdump)

tools/             U-Boot image packaging (packimg, mksquashfs, mk_images.sh, FIT)
scripts/
  fetch-toolchain.sh   downloads the Bootlin mips32 glibc GCC-14 cross toolchain
secrets/           phoebus.env.example — real phoebus.env is gitignored
```

## Use

```sh
scripts/fetch-toolchain.sh          # pulls toolchain/ (not committed)
rootfs/build-rootfs.sh /path/to/rootfs-tree
# normally consumed via submodule from a BSP; see that BSP's build.sh
```

## Userspace

Phoebus-OS runs **s6** as PID 1, not busybox init or procd. See
[`docs/userspace.md`](docs/userspace.md) for the boot flow, the service tree, and
how to add or debug a service.

Credentials (Wi-Fi passphrase, admin password) are **never committed** — this
repo is public. It ships locked accounts and placeholder passphrases; real values
are injected at build time from a gitignored file. See
[`PROVISIONING.md`](PROVISIONING.md).

## Target

RTL9607C/Cv2, BE MIPS32r2, MIPS_CPS + MT SMP (4 VPEs), GIC, kernel load
`0x80001000`. Boot/flash via U-Boot (`Phoebus#`), NAND+UBI.
