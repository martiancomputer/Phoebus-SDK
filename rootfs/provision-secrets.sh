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

# --- admin hash -> /etc/shadow (unlock admin, keep root locked) ---
DAY=$(( $(date +%s) / 86400 ))
{ printf 'root:!:%s:0:99999:7:::\n' "$DAY"
  printf 'admin:%s:%s:0:99999:7:::\n' "$ADMIN_HASH" "$DAY"; } > "$ROOT/etc/shadow"
chmod 0600 "$ROOT/etc/shadow"

echo "provision: stamped Wi-Fi passphrase + admin hash into $ROOT"
