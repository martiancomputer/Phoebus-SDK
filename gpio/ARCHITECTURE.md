# Event-driven board LED policy

The daemon drives **existing LED-class brightness attributes**, not GPIO
registers. The device tree remains the wiring authority, including polarity and
the 200 Hz GPIO-backed PWM period. The existing boot LED driver and its
flash/off/power-blink sequence are unchanged.

One C process blocks in epoll. RTNETLINK, two persistent hostapd datagram
connections, an inotify descriptor, a local seqpacket listener, signalfd and a
single one-shot timerfd feed the state machine. No fork, exec, shell, helper
worker or periodic sysfs network scan occurs in this process.

## Sources and synchronization

| Source | Meaning |
|---|---|
| RTNETLINK link multicast + initial link dump | LAN socket carrier and selected WAN carrier |
| IPv4 address multicast + initial address dump | WAN address present/lost, including daemon restart |
| hostapd ATTACH, initial STATUS and AP events | actual radio readiness, disable and termination |
| inotify + bounded reconnect backoff | hostapd socket creation/replacement; no AP-state polling |
| protected s6 run/finish scripts | radio starting/stopped; independent of LED availability |
| udhcpc callback | bound/renew/deconfig notifications |
| udhcpd notify_file | lease-file update; compare absolute expiry to suppress periodic rewrites |
| root-owned VPN service callbacks | connecting/connected/disconnected; last state retained in tmpfs |
| s6-hpd net-device add/remove hook | advisory dump resynchronization; not duplicate carrier polling |

LAN1–4 map to `eth0.2`–`eth0.5`. The internal SGMII1 interface is deliberately
not treated as a fifth LAN socket. WAN comes from the protected runtime
`wan_iface` selection (allowlisted interface names; default `eth0.8`). No VLAN,
PHY, forwarding, address, DHCP or accelerator configuration is changed.

The kernel handoff uses existing `userspace_ready`/`phase` attributes. Only the
initial handoff has a bounded 50 ms retry, capped at two seconds, so userspace
does not overwrite the initial flash/off sequence. After handoff there are no
LED-state reads: cached brightness avoids redundant writes. Missing or failed
LED attributes disable only that output and set a status failure bit.

## Timers and ordering

Only active animations have deadlines. All indicators progress independently;
there is no serial animation script. Steady states have no animation timer.
Configuration durations are 25–10000 ms, brightness 0–255, LAN count 1–4 or
`port_number`. Fast failure blinking is 100 ms per edge.

New LAN carrier announces that port once with numbered pulses. Another newly
connected port preempts the previous burst. Existing links never reannounce on
duplicate notifications. Removing an older port does not preempt the latest
port's burst. Removing the selected port cancels its burst and leaves the LED
solid if another link remains. All-down is off. A real lease ACK attributed by
bridge FDB to the selected wired port causes one short off/on tick after the
connection burst and a one-second steady interval. Static-addressed clients
correctly produce link indication but no DHCP tick.

WAN no-carrier has highest priority and extinguishes both color channels. With
carrier, the default progresses blinking orange → solid orange on DHCP/address
acquisition → green on successful connectivity. A native 16-byte ICMP echo to
the agreed connectivity endpoint runs every 30 seconds **only** with carrier
and an address. This is a connectivity test, not repeated enumeration of network
state. One outstanding probe has a 1.5-second deadline; three consecutive failures
are required to revoke an online state. No ping subprocess exists. An unavailable
raw socket never yields a false green indication. No packet capture is performed.

Service lifecycle events cannot falsely assert AP-ready. Only hostapd ENABLED
does so. Protected VPN services must emit their own semantic connected state;
an up tunnel interface alone is not evidence of an authenticated connection.
The root client preserves its latest VPN event in `/run` even if the daemon is
temporarily unavailable. That state survives daemon restarts, but not a router
reboot. Producers must report their own teardown; custom binary API producers
must replay their authoritative state after reconnecting.

## Integration and validation

The `gpio-daemon` s6 service has **no network dependency**, and no network service
depends on it. It is started concurrently in `ok-all`. DHCP and hotplug hooks
fail open for indicator delivery. Stopping/crashing this process cannot execute
any network configuration operation. s6 restarts it; startup reconstructs links,
WAN addressing, hostapd readiness and prior VPN notification state.

`make` builds host binaries; pass cross CC/STRIP to `make install DESTDIR=...`
for MIPS. The rootfs and BSP userspace build paths both install the daemon/client.
Run `python3 tests/run.py` for host fake-LED/mock-hostapd tests, including the
same syscall sandbox, and ASan/UBSan state-machine/parser tests. Test-only path
redirection requires a separate `GPIO_TEST` compilation and is absent from
production binaries.

After image generation, `python3 tests/check-image.py <initramfs_data.cpio>
<staged-rootfs>` checks embedded MIPS executables, current file contents,
root ownership, hook permissions and absence of the retired polling service.

Physical validation still required: complete boot handoff, port insert/remove,
DHCP attribution on the target, target seccomp ABI, Wi-Fi restarts, daemon kill/
restart, idle RSS/CPU on MIPS, and throughput/latency under simultaneous WAN/LAN
load and LED animation. Host measurements are not board measurements.
