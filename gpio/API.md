# GPIO event API, version 1

`gpio-daemon` is a foreground, s6-supervised LED service. `gpioctl` is its
unprivileged client, not a setuid program. Neither accepts arbitrary GPIOs,
registers, commands, producer identities or runtime paths.

## Commands

```sh
gpioctl status
gpioctl events
gpioctl identify lan 500
gpioctl reload                         # root
gpioctl event wifi_2g starting          # protected s6 hook, root
gpioctl event wifi_2g stopped           # protected finish hook, root
gpioctl event internet bound           # DHCP callback, root
gpioctl event internet lost
gpioctl event vpn starting             # authorized VPN service, root
gpioctl event vpn ready
gpioctl event vpn stopped
```

Targets: `power`, `lan`, `internet`, `wifi_2g`, `wifi_5g`, `vpn`.
`identify` illuminates one indicator for 100–3000 ms, then restores its current
authoritative pattern. Two requests per UID per 10 seconds are permitted;
overlapping identification on the same target is rejected. WAN no-carrier
always overrides identification, so a disconnected WAN stays dark.

`reload` and SIGHUP reread `/etc/phoebus/gpio.conf`. Use a root-owned, regular
file, mode 0640 or 0600, in the root-owned non-writable configuration directory.
Prepare updates in a temporary root-owned file and rename it atomically. The
entire configuration is validated before it replaces the current one; invalid
files, symlinks, unknown keys/actions and out-of-range values are rejected.
`restore_on_reload=true` reapplies current state with new patterns; `false`
keeps in-progress pattern phases and applies new policy on later transitions.
Brightness is updated immediately in either case.

The native `gpio-lease-notify` symlink invokes the client in udhcpd callback
mode. It ignores udhcpd's filename argument; the daemon reads only its fixed
lease file and bridge FDB. It is **not** a lease-file polling worker.

Lifecycle event clients have a 200 ms response deadline and exit successfully
if the daemon is absent, denied, busy or restarting. They are advisory to the
network service. Interactive commands report errors normally.

## Binary protocol

Transport: AF_UNIX / SOCK_SEQPACKET at `/run/phoebus/gpio.sock`, mode 0666 in a
root-owned 0755 directory. Permission to connect does not grant producer access.
Each packet is exactly the 64-byte `struct gpio_msg` in `protocol.h`:

| Field | Type | Request meaning |
|---|---|---|
| magic | u32 | `0x47504931` |
| version | u16 | 1 |
| op | u16 | STATUS=1, IDENTIFY=2, RELOAD=3, SUBSCRIBE=4, EVENT=5 |
| target | u32 | target enum, zero for STATUS/RELOAD/SUBSCRIBE |
| arg | u32 | milliseconds or event enum; otherwise zero |
| flags | u32 | reserved, must be zero |
| status | i32 | zero in requests; negative errno in replies |
| data | 10 × u32 | reserved in requests, all zero |

Fields are native-endian because the protocol is local-only. No ABI pointers,
paths or variable-length strings occur. Reply op is STATUS=1 or UPDATE=6.
Data words are: LAN bitmask, most recently connected LAN port, WAN flags
(carrier=1, address/lease=2, successful connectivity=4), six target states in
target enum order, and a seven-bit hardware-LED failure mask. State enums are
starting=0, ready=1, stopped=2, failed=3, DHCP flash=4, online=5, lost=6.

Root-only events: STARTING=1, READY=2, STOPPED=3, FAILED=4 for power/radios/VPN;
WAN_BOUND=5 and WAN_LOST=6 for Internet; LEASES=7 for LAN; RESYNC=8 for power.
Radio READY requests do not assert readiness: the daemon still requires the
connected hostapd control socket to report ENABLED.

Ordinary users may read, subscribe and identify only. Identity comes exclusively
from SO_PEERCRED. Truncated/oversized messages, ancillary descriptors, nonzero
reserved fields, unknown versions/opcodes and invalid event/target pairs fail.

Limits: 32 clients total, 4 per UID, 8 subscriptions, 32 UID rate buckets;
20 requests/10 seconds per ordinary UID, 200 for root. Idle non-subscribers close
after 3 seconds. Replies are one packet; there is no userspace output queue.
Slow subscribers are disconnected when the bounded socket send buffer fills.
