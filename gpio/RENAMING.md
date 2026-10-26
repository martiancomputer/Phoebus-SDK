# Functional naming scheme

Use service/function names for userspace and short functional filenames for
the board's LED and hardware-monitor helpers. Keep hardware/subsystem prefixes
where useful for kernel symbols. Do not rename every symbol just for consistency:
public interfaces and device-tree compatible strings require compatibility.
The GPIO replacement and the five explicitly selected renames are implemented.
Other entries remain proposals; existing kernel interfaces and DT compatibles
are deliberately unchanged.

| Existing name | Proposed name | Scope |
|---|---|---|
| `phoebus-leds`, service `leds` | `gpio-daemon`, service `gpio-daemon` | Implemented native replacement |
| `phoebus-led-event` | `gpioctl event` | Implemented bounded, authenticated client |
| `phoebus-led-lease` | `gpio-lease-notify` (native gpioctl mode) | Implemented DHCP callback |
| `phoebus-kmsg-relay` | `kmsg-relay` | Implemented userspace binary/source rename; service already named this |
| `phoebus-check` | `sys-check` | Implemented health-check rename |
| `phoebus-diag` | `sys-diag` | Implemented diagnostic-tool rename |
| `phoebus-telemetry.conf` | `/etc/telemetry.conf` | Protected logging configuration and all consumers |
| `phoebus_led_boot.c` | `leds-boot.c` | Implemented kernel boot LED filename rename |
| `phoebus_hwmon.c` | `hwmon.c` | Implemented kernel hardware-monitor filename rename |
| `phoebus-spinand-ro` driver label | `rtl9607c-spinand-ro` | Existing hardware-specific NAND filename already suitable |
| `phoebus_wan` kernel worker | `rtl9607c_wan` | Hardware-specific kthread label |
| `phoebus_rf` diagnostic endpoint | `rf_status` | Driver diagnostic API, needs consumer migration |
| `phoebus_poke` endpoint | retire from production; gated debugfs register interface if needed | Do not expand raw register access while renaming |

Keep `rtk_fc`, `hostapd`, `udhcpd`, `dnsmasq`, `dropbear`, `s6-hpd` and existing
subsystem names: they already identify their function or upstream component.
Use `rtl9607c_*`/`rtl8192cd_*` for relevant kernel symbols, not a branding prefix.

The GPIO task explicitly retains `/etc/phoebus/gpio.conf` and
`/run/phoebus/gpio.sock`. If removing that namespace is desired later, migrate
them together to `/etc/gpio-daemon/gpio.conf` and `/run/gpio-daemon/control.sock`
with a documented transition. Do not silently rename shipped DT compatible
strings, sysfs/proc interfaces, module parameters or boot arguments; preserve
aliases where consumers or old firmware rely on them.
