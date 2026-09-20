#!/bin/sh
# Host-side helper regression. No router, GPIO, NAND or networking is touched.
set -eu
TEST_HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
DIAG="$TEST_HERE/../rootfs/usr/bin/phoebus-diag"
TEST_TMP=$(mktemp -d)
trap 'rm -r -- "$TEST_TMP"' EXIT HUP INT TERM
cp -R "$TEST_HERE/fixtures/board-diag/." "$TEST_TMP/"
export PHOEBUS_DIAG_PROC="$TEST_TMP/proc" PHOEBUS_DIAG_SYS="$TEST_TMP/sys"
PROBE="$PHOEBUS_DIAG_PROC/fc/ctrl/phoebus_led_probe"
sh -n "$DIAG"
sh "$DIAG" snapshot > "$TEST_TMP/snapshot"
for expected in 'readonly=1' 'hwnat_mode=4' 'bypass_eapol=1' 'last_restore_rc=0'; do
    grep -Fq "$expected" "$TEST_TMP/snapshot"
done
sh "$DIAG" led 03 on 0050 > /dev/null
[ "$(cat "$PROBE")" = 'led 3 on 50' ]
sh "$DIAG" gpio 95 low > /dev/null
[ "$(cat "$PROBE")" = 'gpio 95 low 500' ]
before=$(cksum "$PROBE")
invalid() {
    if sh "$DIAG" "$@" > /dev/null 2>&1; then
        echo 'FAIL: accepted invalid arguments' >&2; exit 1
    fi
    [ "$(cksum "$PROBE")" = "$before" ]
}
invalid led 18 on
invalid gpio 96 low
invalid led -1 on
invalid led 1 blink
invalid led 1 on 49
invalid led 1 on 3001
invalid led 1 on 999999999999999999999999
invalid led '1;touch BAD' on
invalid led 1 on 500 extra
invalid snapshot extra
echo 'PASS: mocked snapshot, decimal normalization, default duration, rejected input without probe writes'
