# GPIO daemon security boundaries

Threat model: hostile local unprivileged clients, malformed IPC/configuration,
event storms, slow subscribers, missing LED hardware and daemon restarts. Root
and protected service scripts remain trusted. This is not a sandbox against an
already-compromised root service, nor a replacement for network firewall rules.

## Authority and access

- Hardware paths and the seven LED labels are compiled constants. Configuration
  changes effects only, never GPIO numbers, paths, producer registration or code.
- SO_PEERCRED is checked on accepted sockets. There are no client-supplied user
  IDs or producer credentials. Only UID 0 may reload policy or inject semantic
  service state. Rootless users get status, bounded identify and subscriptions.
- Socket directory is root-owned and not group/world writable. Config directory
  and file ownership are checked; the file must be regular, non-writable by
  other users, non-symlink and single-link, at most 8192 bytes. Parsing occurs
  into a temporary structure; a failed reload leaves the active policy intact.
- IPC is fixed-size and versioned, with exact bounds and reserved-bit checks.
  Ancillary data and passed descriptors are never consumed. Connection, UID,
  request, identification and output-buffer limits bound local resource use.

## Privilege reduction

The supervised process starts as root to open allowlisted LED attributes, its
control sockets and one ICMP socket. It retains UID 0 solely for owner-only
hostapd reconnection, configuration reload and fixed tmpfs state, but clears
**all permitted/effective/inheritable Linux capabilities** after initialization.
It applies `no_new_privs`, disables core dumps and installs an architecture-
checked seccomp syscall allowlist. There is no exec, fork/clone, ioctl, ptrace,
mount, module loading or reboot syscall. Unknown syscalls fail with EPERM.
Failure to establish these protections is an explicit initialization failure;
no unsandboxed fallback is silently used. The BSP enables SECCOMP/FILTER for
the target image; this daemon must not be deployed onto an older image lacking
that support without an explicit compatibility decision.

Keeping UID 0 is a deliberate limitation, not a claim of full unprivileged
execution. Dropping to a dedicated UID would additionally require carefully
delegated hostapd socket/config access. The syscall sandbox permits some file
and socket operations needed for reconnection/reload; it does not enforce paths.
The audited fixed-path application code is part of the security boundary.

## Network and storage separation

The daemon never changes routes, VLANs, interface flags, radio parameters,
firewall/NAT policy or accelerator settings. Its RTNETLINK use is receive plus
GET dumps only. Kernel netlink peers and connected hostapd sockets are the
authority for links and AP-ready. Native ICMP replies must match endpoint,
identifier, sequence and unpredictable per-process nonce. Three failures give
hysteresis, but blocking ICMP can still make a working Internet connection look
unavailable; this affects the LED only.

Only LED attributes and fixed runtime objects are written. The configuration
is read-only to the daemon. There are no NAND, eFuse, raw GPIO, MDIO, PCIe,
UART, arbitrary register or plugin paths. VPN state and sockets live in tmpfs.
The client cannot change event-source registration through the pattern config.

Host tests cover malformed packets, ancillary rights rejection, real rootless
producer denial, synthetic authorization/rate-limit cases, atomic reload,
symlink rejection, parser/state-machine sanitizer checks and syscall denial.
They are not an exhaustive security audit or proof of target/runtime safety.
