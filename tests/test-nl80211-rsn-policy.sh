#!/bin/sh
# Check the real header's feature guards without kernel/platform includes.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
HEADER=$HERE/../vendor/realtek-wireless/rtl8192cd/8192cd_cfg.h
detect=$(awk '/^#if defined\(CONFIG_CFG80211\) \|\| defined\(CONFIG_CFG80211_MODULE\)/,/^#endif/' "$HEADER")
policy=$(awk '/^#if defined\(RTK_NL80211\) \|\| defined\(CONFIG_RTL_MULTI_WIFI_MODULES\)/,/^#endif/' "$HEADER")
[ -n "$detect" ] && [ -n "$policy" ]
for option in CONFIG_CFG80211 CONFIG_CFG80211_MODULE CONFIG_RTL_MULTI_WIFI_MODULES; do
	macros=$(printf '%s\n%s\n' "$detect" "$policy" |
		"${CC:-cc}" -E -dM -x c -D"$option"=1 -)
	printf '%s\n' "$macros" | grep -q '^#define DIRECT_HAPD_RSN_IE'
done
macros=$(printf '%s\n%s\n' "$detect" "$policy" | "${CC:-cc}" -E -dM -x c -)
if printf '%s\n' "$macros" | grep -q '^#define DIRECT_HAPD_RSN_IE'; then
	echo 'FAIL: legacy non-nl80211 configuration changed' >&2
	exit 1
fi
echo 'PASS: built-in/module nl80211 and legacy multi-radio preserve hostapd RSN; other legacy builds unchanged'
