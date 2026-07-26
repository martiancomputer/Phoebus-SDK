# Provisioning credentials

This repo is **public**, so it ships no real secrets:

- `rootfs/etc/shadow` — both `root` and `admin` are **locked** (`*`). No password
  login works until you provision.
- `rootfs/etc/hostapd*.conf` — carry a public placeholder `wpa_passphrase`.

Real credentials live in a gitignored file and are injected into the *built*
rootfs tree, so they end up in your image but never in git.

## One-time setup

```sh
cp secrets/phoebus.env.example secrets/phoebus.env
# edit secrets/phoebus.env: set WIFI_PASSPHRASE and ADMIN_PASSWORD
```

`secrets/phoebus.env` is listed in `.gitignore`; keep it that way.

## Build + provision

```sh
# 1. build the rootfs tree (busybox + s6 + tools)
rootfs/build-rootfs.sh /path/to/rootfs-tree

# 2. stamp real credentials into it
rootfs/provision-secrets.sh /path/to/rootfs-tree

# 3. pack the tree into the image (BSP-side)
```

`build-rootfs.sh` calls `provision-secrets.sh` automatically at the end if
`secrets/phoebus.env` exists; step 2 is only needed when provisioning a tree by
hand. With no secrets file the build still succeeds — the image just keeps the
locked accounts and placeholder Wi-Fi.

## Accounts

- **SSH:** `ssh admin@192.168.1.1` — password auth against `/etc/shadow`.
  `root` SSH is refused two ways: the account is locked *and* dropbear runs with
  `-w` (no root login). Console (serial UART) still gives a direct root shell for
  physical recovery.
- **Host key:** dropbear generates an ed25519 host key on first boot. The rootfs
  is a RAM initramfs, so it regenerates each boot until the board has persistent
  storage — expect an SSH "host key changed" prompt across reboots.

## Wi-Fi

Both radios run **WPA2/WPA3 transition** (`wpa_key_mgmt=WPA-PSK SAE`): WPA3-SAE
where the client and driver support it, WPA2-PSK otherwise. Same passphrase on
`Phoebus5` (5 GHz) and `Phoebus2.4` (2.4 GHz).
