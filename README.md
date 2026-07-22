# Phoebus-SDK

Shared, **kernel-version-independent** support code for Realtek Phoebus
(RTL9607C / RTL9607Cv2 — big-endian MIPS32r2 interAptiv) board support packages.

Consumed as a git submodule by the per-kernel BSPs:

| Repo | Kernel line |
|------|-------------|
| [PhoebusBSP-6](https://github.com/martiancomputer/PhoebusBSP-6) | LTS (currently 6.18.y) |
| [PhoebusBSP-7](https://github.com/martiancomputer/PhoebusBSP-7) | mainline / bleeding edge |

Everything here is the **pristine** SoC truth as extracted from the vendor GPL
drop. It is *not* adapted to any particular kernel — each BSP carries its own
patch series that ports this code to its kernel version. That keeps one copy of
the ~120 MB vendor SDK serving every kernel line, and makes each port a
reviewable diff.

## Layout

```
vendor/
  realtek-net/     full Realtek drivers/net/ethernet/realtek vendor overlay, as-shipped:
                   rtl86900 (switch + GPON/EPON + NIC + FleetConntrack), plus the other
                   selectable PHY/switch drivers (rtl82xx, rtk-ms-phyOnly, rtl8367, ...)
                   and rtk_* helpers. Upstream-owned files (r8169, 8139*, atp) excluded.
  platform/        MIPS BSP: arch/mips/rtl9607c, dts/realtek, mach-rtl960xc headers,
                   drivers/{clk/realtek,gpio/gpio-rtk-soc.c,watchdog/rtl819x_wdt.c}
  include/         vendor headers: net/rtl, soc/cortina, dt-bindings/soc/9607xc_irqs.h
tools/             U-Boot image packaging (packimg, mksquashfs, mk_images.sh,
                   phoebus FIT) — bootloader/flash format, kernel-version-independent
rootfs/            busybox config + /etc skeleton + build-rootfs.sh
scripts/
  fetch-toolchain.sh   downloads the Bootlin mips32 glibc GCC-14 cross toolchain
```

## Use

```sh
scripts/fetch-toolchain.sh          # pulls toolchain/ (not committed)
# consumed via submodule from a BSP; see that BSP's build.sh
```

Target: RTL9607C/Cv2, BE MIPS32r2, MIPS_CPS + MT SMP (4 VPEs), GIC, kernel load
`0x80001000`. Boot/flash via U-Boot (`Phoebus#`), NAND+UBI.
