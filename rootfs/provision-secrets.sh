#!/bin/sh
# Stamp real Phoebus credentials into a BUILT rootfs tree.
#
# The repo ships only placeholders: /etc/shadow has both accounts LOCKED and the
# hostapd configs carry a public dummy passphrase. This script injects the real
# Wi-Fi passphrase and admin password hash from a gitignored secrets file, so the
# secrets live on the builder's disk and in the image -- never in git.
#
#   ./provision-secrets.sh <rootfs-tree> [secrets-env]
#
# secrets-env defaults to ../secrets/phoebus.env and must define WIFI_PASSPHRASE
# plus either ADMIN_HASH (a crypt(3) string, used verbatim) or ADMIN_PASSWORD
# (hashed here with `openssl passwd -6`).
#
# If the secrets file is absent this is a no-op (image keeps locked accounts +
# placeholder Wi-Fi) so CI without secrets still builds.
set -e
ROOT="$1"
ENVF="${2:-$(cd "$(dirname "$0")/.." && pwd)/secrets/phoebus.env}"

[ -n "$ROOT" ] && [ -d "$ROOT" ] || { echo "usage: $0 <rootfs-tree> [secrets-env]" >&2; exit 1; }
if [ ! -f "$ENVF" ]; then
	echo "provision: no secrets at $ENVF -- keeping LOCKED accounts + placeholder Wi-Fi." >&2
	echo "provision: see PROVISIONING.md to supply real credentials." >&2
	exit 0
fi

# Read WITHOUT sourcing: a $6$... hash would be mangled by shell expansion.
WIFI_PASSPHRASE=$(sed -n 's/^WIFI_PASSPHRASE=//p' "$ENVF")
ADMIN_PASSWORD=$(sed -n 's/^ADMIN_PASSWORD=//p' "$ENVF")
ADMIN_HASH=$(sed -n 's/^ADMIN_HASH=//p' "$ENVF")

[ -n "$WIFI_PASSPHRASE" ] || { echo "provision: WIFI_PASSPHRASE missing in $ENVF" >&2; exit 1; }
if [ -z "$ADMIN_HASH" ]; then
	[ -n "$ADMIN_PASSWORD" ] || { echo "provision: need ADMIN_HASH or ADMIN_PASSWORD in $ENVF" >&2; exit 1; }
	ADMIN_HASH=$(openssl passwd -6 "$ADMIN_PASSWORD")
fi

# --- Wi-Fi passphrase -> every hostapd*.conf ---
# `|` delimiter dodges `/` in passphrases; keep `|`, `&`, `\` out of the passphrase.
for c in "$ROOT"/etc/hostapd.conf "$ROOT"/etc/hostapd-2g.conf; do
	[ -f "$c" ] || continue
	sed -i "s|^wpa_passphrase=.*|wpa_passphrase=$WIFI_PASSPHRASE|" "$c"
done

# --- hashes -> /etc/shadow ---
# admin (uid 1000) is the SSH account and dropbear runs with -w, so root can
# never log in over the network regardless. root still needs a usable password
# though: with the old `root:!` entry `su` could never succeed, which left the
# "admin" account unable to administer anything -- it could not even restart
# hostapd ("s6-rc: fatal: unable to take locks: Permission denied"). Defaults to
# the admin hash so one password covers both; set ROOT_HASH to separate them.
ROOT_HASH=$(sed -n 's/^ROOT_HASH=//p' "$ENVF")
[ -n "$ROOT_HASH" ] || ROOT_HASH="$ADMIN_HASH"

DAY=$(( $(date +%s) / 86400 ))
{ printf 'root:%s:%s:0:99999:7:::\n' "$ROOT_HASH" "$DAY"
  printf 'admin:%s:%s:0:99999:7:::\n' "$ADMIN_HASH" "$DAY"; } > "$ROOT/etc/shadow"
chmod 0600 "$ROOT/etc/shadow"

# --- optional SSH public keys -> admin's authorized_keys ---
#
# The rootfs is a RAM initramfs, so anything added to it on a running board is
# gone at the next boot -- unlike /etc/shadow, which is baked in here. Pasting a
# key over the serial console after every flash is the kind of manual step that
# gets skipped exactly when the board is already broken and SSH is the only way
# in, so keep it in the image.
#
# ADMIN_SSH_KEYS in the secrets file, one key per line (use a quoted multi-line
# value, or a single key). Public keys are not secret, but they live in the
# gitignored secrets file rather than the tracked tree on purpose: this repo is
# public, and baking a key into every image anyone builds would hand out access
# to whoever holds the matching private half.
ADMIN_SSH_KEYS=$(sed -n 's/^ADMIN_SSH_KEYS=//p' "$ENVF" | sed 's/^"//; s/"$//')
if [ -n "$ADMIN_SSH_KEYS" ]; then
	mkdir -p "$ROOT/home/admin/.ssh"
	printf '%s\n' "$ADMIN_SSH_KEYS" > "$ROOT/home/admin/.ssh/authorized_keys"
	chmod 0700 "$ROOT/home/admin/.ssh"
	chmod 0600 "$ROOT/home/admin/.ssh/authorized_keys"
	# initramfs is generated with ROOT_UID/GID=0, so ownership is fixed up at
	# image build time, not here.
	n=$(grep -c . "$ROOT/home/admin/.ssh/authorized_keys" 2>/dev/null || echo 0)
	echo "provision: installed $n SSH key(s) for admin"
fi

echo "provision: stamped Wi-Fi passphrase + admin/root hashes into $ROOT"
