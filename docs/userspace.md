# Phoebus-OS userspace

Phoebus-OS runs the **s6** supervision suite as PID 1 — not busybox init, not
procd. This document covers the boot flow, the service tree, and how to work on
it.

## Why s6

procd is excellent but is welded to OpenWrt's ubus/uci world. sysvinit and
busybox init have no supervision at all. s6 gives real process supervision,
dependency-ordered startup, and a readiness model, in ~10 MB of small static-ish
binaries — a good fit for a RAM-booted router.

What s6 *doesn't* have is procd's hotplug handling, so we wrote that piece:
[`s6-hpd`](../s6-hpd/s6-hpd.c).

## Boot flow

```
U-Boot ──bootm──► kernel ──► /init  (s6 stage 1, PID 1)
                               │
                               ├─ mount proc/sys/dev/run/tmp/devpts
                               ├─ dmesg -n 4          (quiet the console, see below)
                               ├─ s6-rc-compile /run/s6-rc/compiled /etc/s6/source
                               ├─ cp -a /etc/s6/sv/. /run/service/
                               └─ exec s6-svscan -t0 /run/service   ← PID 1 for the rest of time
                                        │
                                        ├─ getty-console   (supervised shell on ttyS0)
                                        └─ rc-bootstrap    (one-shot)
                                             └─ s6-rc-init + s6-rc -u change ok-all
                                                  └─ the service tree below
```

Two details that matter:

- **The scandir is a tmpfs copy.** `s6-supervise` must create control FIFOs, so
  the live scandir (`/run/service`) is copied from the read-only `/etc/s6/sv`.
- **`getty-console` is a *direct* scandir service**, not part of s6-rc. If the
  whole s6-rc bring-up fails you still get a console. That has saved several
  debugging sessions.

## The service tree

`/etc/s6/source` is the s6-rc source database, compiled on-target at boot.

| Service | Type | Depends on | Does |
|---|---|---|---|
| `network` | oneshot | — | bridge, LAN IP, MAC provisioning |
| `syslogd` / `klogd` | longrun | — | logging |
| `s6-hpd` | longrun | — | hotplug (uevent → handler) |
| `hostapd` | longrun | network | 5 GHz AP (wlan0) |
| `hostapd-2g` | longrun | network | 2.4 GHz AP (wlan1) |
| `udhcpd` | longrun | network | DHCP server on the LAN bridge |
| `wan` | longrun | network | DHCP *client* on the WAN port |
| `nat` | oneshot | wan | masquerade + stateful firewall |
| `dropbear` | longrun | network | SSH server |
| `ok-all` | bundle | — | everything above |

Oneshot scripts live in `etc/s6/scripts/`; the service dirs just invoke them.

### Gotcha: a oneshot's exit code is its last command

s6-rc treats a non-zero return from a oneshot as service failure and **aborts the
whole bundle**. `network-up` originally ended with a carrier-report loop whose
final `[ "$c" = "1" ]` test returned 1 when a port had no cable — so bring-up
"failed" even though the network was fine, and nothing else started. Oneshot
scripts here end with an explicit `exit 0`.

## Working on services

```sh
# what's up
s6-rc -l /run/s6-rc/live -a list

# restart one longrun
s6-svc -r /run/service/hostapd

# stop / start via s6-rc (handles dependencies)
s6-rc -l /run/s6-rc/live -d change hostapd
s6-rc -l /run/s6-rc/live -u change hostapd
```

The rootfs is a **RAM initramfs** — edits on the device are lost at reboot.
Persist them in this repo and rebuild.

## Console noise

The Realtek switch and Wi-Fi drivers are extremely chatty at KERN_INFO/WARN
(`[FCMGR]`, `RTW:`, `PHL:`, `halbb`). At the usual `loglevel=8` that firehose
interleaves with the shell and cuts you off mid-command, so `/init` runs
`dmesg -n 4`.

Nothing is lost — it all still reaches the kernel ring buffer:

```sh
dmesg            # full history regardless of console level
dmesg -n 8       # firehose back on, live
```

Boot with `phoebus_verbose` on the kernel cmdline to keep it verbose from the
start.

## Networking

LAN is a bridge (`br0`, `192.168.1.1/24`) holding the Wi-Fi interfaces and the
wired LAN ports. WAN is a single port kept **out** of the bridge and addressed by
DHCP; `nat` masquerades LAN→WAN with a default-DROP forward policy.

### Do not bridge raw `eth0`

`eth0` is the switch's **CPU port**. Bridging it creates an L2 loop: the switch
floods frames the CPU just sent straight back to it, `br0` relearns its own MAC
on a port, and forwarding blackholes. The console symptom is

```
br0: received packet on eth0 with own address as source address
```

Bridge the per-port VLAN netdevs instead (`eth0.2`–`eth0.7`), each of which is
its own broadcast domain. Port map, from the vendor `re8686_rtl9607c.c`:

```
eth0        CPU port          eth0.2-.7   LAN_PORT1..6
eth0.8/.9   SGMII0/SGMII1     nas0        WAN_PORT
```

Override the bridge members with `lan=<if>[,<if>...]` and the WAN port with
`wan=<if>` on the kernel cmdline. `network-up` prints a carrier report at boot so
you can see which socket a cable is actually in.

## Security posture

- `root` is **locked** (`!` in `/etc/shadow`); `admin` (uid 1000) is the login
  account. dropbear additionally runs with `-w` (no root SSH). The serial console
  still gives root, for physical recovery.
- Wi-Fi runs WPA2/WPA3 transition (`wpa_key_mgmt=WPA-PSK SAE`, `ieee80211w=1`).
- USB host controllers are compiled in but gated off at `bsp_usb_init()` — the
  board has no USB port, so it is attack surface with no upside.
- WPS is deliberately not built into hostapd.
- Real credentials never enter this repo; see [`../PROVISIONING.md`](../PROVISIONING.md).

## Adding a service

```sh
mkdir -p rootfs/etc/s6/source/myservice
echo longrun            > rootfs/etc/s6/source/myservice/type
echo network            > rootfs/etc/s6/source/myservice/dependencies
cat > rootfs/etc/s6/source/myservice/run <<'EOF'
#!/bin/sh
exec myprogram --foreground
EOF
echo myservice >> rootfs/etc/s6/source/ok-all/contents
```

Longruns must run in the **foreground** — if the program daemonises, s6 loses
track of it and restarts it forever. Do not wrap `run` in busybox `setsid`
either: it forks, the tracked parent exits, and s6 respawns endlessly while
orphaned children pile up.
