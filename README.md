# Phoebus-SDK

Shared, kernel-version-independent support for the Realtek **RTL9607C / RTL9607Cv2**
platform used by Phoebus-OS.

This repository is the common layer underneath the two kernel ports:

| Repository | Kernel line | Role |
|---|---|---|
| [PhoebusBSP-6](https://github.com/martiancomputer/PhoebusBSP-6) | Linux 6.18 LTS | hardware-debug / reference line |
| [PhoebusBSP-7](https://github.com/martiancomputer/PhoebusBSP-7) | current mainline, presently 7.3-rc3 | forward-port / API-churn line |

The split is intentional. Kernel-version-specific fixes stay in the BSP overlays.
The SDK carries the shared vendor snapshot, root filesystem, service graph,
provisioning, diagnostics, networking policy and image/tooling code.

## Platform

Current target hardware is the TP-Link Archer AX10 v3 / AX1500 variant built on
the RTL9607C:

- big-endian MIPS32r2 interAptiv
- MIPS CPS + MT SMP, 4 VPEs
- GIC interrupt controller
- 256 MiB RAM
- integrated Realtek switch / xPON block
- RTL8832BR 5 GHz radio, vendor `rtk_wifi6` driver
- RTL8192F 2.4 GHz radio, vendor `rtl8192cd` driver
- 128 MiB SPI-NAND
- stock U-Boot retained as the bootloader

The kernel ports reconstruct a buildable tree from a pristine kernel.org source
tree, the vendor snapshot in this repository, and a small per-kernel overlay.
That makes the kernel delta reviewable instead of maintaining a fork of the
entire vendor tree.

## What has been brought up

The 6.18 LTS line is the hardware reference. On real hardware it has reached:

- 4-way SMP, GIC timers/clocksource, serial console, GPIO and watchdog
- Realtek switch/xPON core, FleetConntrack infrastructure and both PCIe links
- four wired LAN ports at gigabit
- WAN on the external PHY/SerDes path, DHCP, routing and NAT
- both Wi-Fi radios
- WPA2/WPA3 transition mode with SAE on 5 GHz
- 802.11ax on RTL8832BR, 80 MHz operation and 1201 Mbit negotiated PHY rate
- s6-supervised userspace, SSH, DHCP/DNS and a stateful firewall
- bidirectional CAKE/SQM on the WAN
- read-only NAND dump and extraction tooling

The mainline BSP is kept as a forward-port of the same work. It has moved through
7.1 and is currently rebased onto **7.3-rc3**. Mainline API changes are handled in
that repository rather than being mixed into this shared SDK.

Hardware verification and forward-porting are deliberately tracked separately:
a feature being present in BSP-7 means it builds against the current mainline
tree; it does not imply that every forwarded change has already been re-tested
on silicon. See each BSP's `PROJECT.md` for its current verification boundary.

## Userspace architecture

Phoebus-OS uses **s6** as PID 1. It is not OpenWrt and does not use procd or UCI.

Boot flow:

```
U-Boot
  -> Linux + initramfs
      -> /init
          -> s6-rc-compile
          -> s6-svscan
              -> rc-bootstrap
                  -> s6-rc change ok-all
```

The current `ok-all` service bundle contains:

```
network
syslogd
klogd
s6-hpd
hostapd
hostapd-2g
udhcpd
wan
nat
dropbear
dnsmasq
cake
wifi-tune
wifi-acs
```

`s6-hpd` is a small NETLINK_KOBJECT_UEVENT daemon written for this project. It
fills the hotplug role that procd/udev normally provides while remaining inside
the s6 supervision model.

The rootfs builder currently uses BusyBox **1.38.0**, builds the complete s6
runtime, installs the regulatory database required by cfg80211, carries the
tracked `/etc` and `/usr` skeletons, and emits a manifest for third-party
binaries so incomplete images are visible at build time.

### Third-party userspace

The SDK owns configuration and service integration for hostapd, dnsmasq,
Dropbear, iptables, `tc`, wireless_tools and the diagnostic utilities.

Reproducibility differs between the two BSPs:

- **BSP-7** has a userspace build pipeline for the third-party networking stack.
- **BSP-6** still depends on several binaries being staged from external source
  trees; its rootfs build reports missing components rather than pretending the
  image is complete.

This distinction matters when comparing a clean clone with a previously staged
development tree.

## Networking

The LAN is `br0`; raw `eth0` is the switch CPU port and must not be bridged.
The physical sockets are exposed through the per-port VLAN netdevs.

Measured board map on the reference hardware:

```
LAN1 -> eth0.2      LAN3 -> eth0.4
LAN2 -> eth0.3      LAN4 -> eth0.5
WAN  -> eth0.8      switch port 6 / SDS0
```

The WAN service runs DHCP, `nat` installs a default-drop forwarding policy with
MASQUERADE and MSS clamping, and `wanmac=` can optionally clone only the WAN
MAC without disturbing the bridge or radio BSSIDs.

The Realtek flow accelerator is intentionally disabled for the current topology.
On this board the offload path cannot correctly represent the PCIe Wi-Fi path
and eventually overflows the vendor device stack. Software forwarding on the
LTS line has been measured around 350 Mbit/s router-terminated.

### CAKE

CAKE is applied in both directions:

- root CAKE qdisc on WAN egress
- IFB redirect for ingress shaping
- NAT-aware host fairness
- hardware flow offload kept disabled so packets remain visible to the qdisc

On a 100 Mbit test line the reference configuration shapes at roughly 92 Mbit/s
and was measured at about 93/94 Mbit/s while removing the several-hundred-ms
loaded latency seen with the stock firmware.

## Wireless

The two radios are unrelated driver stacks and are treated independently.

### 5 GHz — RTL8832BR / `rtk_wifi6`

Implemented in the shared userspace:

- hostapd + nl80211 AP mode
- WPA2/WPA3 transition mode and SAE
- 802.11ax enabled
- 80 MHz channel width
- signed regulatory database installed in the image
- `iwpriv` support for the vendor private MIB
- minimal transmit-side tuning via `wifi-tune`
- automatic channel selection via `wifi-acs`

`wifi-acs` only scans when the radio is idle and no client is actively awake,
scores non-DFS 80 MHz blocks, applies hysteresis, and switches with a hostapd
Channel Switch Announcement rather than restarting the AP.

The remaining LTS-line wireless defect is on the **5 GHz transmit/downstream
path**: upstream can run cleanly while AP-to-client traffic shows heavy retries
and rate-control instability. That is still an active hardware/driver issue and
is not hidden by the README.

### 2.4 GHz — RTL8192F / `rtl8192cd`

The 2.4 GHz radio is an 802.11n path with its own hostapd instance. A major
latency/SSH defect was traced to bringing up nine unused vendor VAP interfaces:
the driver counted them as active BSSes and forced a 400 TU beacon interval.
The network bring-up now leaves unused VAP/WDS/vxd devices down, restoring the
expected 100 TU beacon interval.

## Provisioning and SSH

No real credentials are committed.

`secrets/phoebus.env` is gitignored and can inject at build time:

- Wi-Fi passphrase
- admin password/hash
- optional SSH public keys

The same provisioned public keys may be installed for root. In the development
image Dropbear permits **root by public key only** (`-g`) because the Realtek
private MIB setters require root; root password authentication remains disabled
over SSH. Admin password/key login remains available for normal access.

The initramfs is volatile, so host keys and runtime edits do not survive a
reboot unless they are baked into the next image.

## NAND and diagnostics

Persistent installation is intentionally not enabled yet.

The tooling is designed around observation first:

- `tools/nand-dump.sh` — read NAND from the board
- `tools/nand-extract.sh` / `tools/ubi-unpack.py` — offline extraction
- `phoebus-diag` — board telemetry for NAND, LEDs/GPIO and switch/FleetConntrack
  state
- host-side fixture tests for the diagnostic helper

The current mainline BSP also contains a **read-only** RTL9607C SPI-NAND MTD
driver. It exposes the stock NAND geometry/ECC path without program, erase,
markbad or automatic UBI attachment. That is a bring-up/validation step, not a
flash-install path.

## Repository layout

```
vendor/
  realtek-net/       switch, NIC, xPON, FleetConntrack and PHY/switch support
  realtek-wireless/  RTL8832BR and RTL8192F vendor wireless trees
  platform/          RTL9607C MIPS platform, DTS, clock/GPIO/watchdog code
  include/           vendor headers and dt-bindings

rootfs/
  build-rootfs.sh        BusyBox + s6 + tracked rootfs + manifest
  provision-secrets.sh  build-time credential/key injection
  init                  s6 stage-1 bootstrap
  etc/s6/               s6-rc service database and scripts
  usr/bin/phoebus-diag  on-target diagnostic helper

s6/                  cross-build of skalibs/execline/s6/s6-rc/s6-linux-init
s6-hpd/              supervised uevent/hotplug daemon
ap/                  hostapd build notes
net/                 network userspace notes + tc build helper
tools/               U-Boot image, TFTP, NAND and UBI tooling
tests/               host-side diagnostic regression tests
scripts/             cross-toolchain fetch helper
secrets/             public template; real environment file is ignored
```

## Building

Normally build through one of the BSP repositories so the kernel, rootfs and
image are produced together:

```sh
git clone --recurse-submodules https://github.com/martiancomputer/PhoebusBSP-6
# or PhoebusBSP-7
cd PhoebusBSP-*
./build.sh
```

The SDK can also build its rootfs layer directly:

```sh
scripts/fetch-toolchain.sh
rootfs/build-rootfs.sh /path/to/rootfs-tree
```

Read the manifest printed at the end of `build-rootfs.sh`. A successful shell
exit is not proof that hostapd, Dropbear, dnsmasq, iptables, `tc` or
wireless_tools were actually staged.

## Engineering rules

A few rules are enforced by experience rather than style preference:

- verify the generated artefact, not only the build exit status
- keep hardware-debug fixes on the LTS line, then forward-port them
- do not infer behaviour of one Wi-Fi driver from the other
- do not treat a successful compile as hardware verification
- keep NAND access read-only until ECC, bad-block handling and offsets are
  independently validated
- keep the vendor snapshot shared and kernel-specific changes in BSP overlays
- treat clean-clone reproducibility as a feature, not a cleanup task

The detailed hardware findings and current open defects live in the BSP
`PROJECT.md` files; this README describes the shared engineering layer and the
state that both kernel lines build on.
