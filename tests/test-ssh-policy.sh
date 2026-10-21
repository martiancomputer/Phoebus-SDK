#!/bin/sh
# Exercise policy generation without changing the host firewall.
set -eu
if [ "${SSH_POLICY_MOCK:-}" = 1 ]; then
	case "${0##*/}" in
		ip)
			case "$1" in
				-4) echo '    inet 192.0.2.1/24 scope global lan0' ;;
				-6) echo '    inet6 2001:db8:1::1/64 scope global' ;;
			esac ;;
		iptables|ip6tables)
			[ "$1 $2" = '-w 10' ]
			if [ "$3" = '-S' ]; then
				[ "${SSH_POLICY_LIST_RC:-0}" = 0 ] || exit "$SSH_POLICY_LIST_RC"
				if [ "${SSH_POLICY_EXISTING:-1}" = 0 ]; then
					case "${0##*/}" in
						iptables) echo '-N PHOEBUS_SSH4' ;;
						ip6tables) echo '-N PHOEBUS_SSH6' ;;
					esac
				fi
				exit 0
			fi
			# Match legacy first-boot behavior: an unknown jump target is 2.
			exit "${SSH_POLICY_EXISTING:-2}" ;;
		iptables-restore|ip6tables-restore)
			[ "$*" = '-w 10 --noflush' ]
			cat >> "$SSH_POLICY_CAPTURE/${0##*/}.rules" ;;
	esac
	exit 0
fi
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
POLICY=$HERE/../rootfs/etc/s6/scripts/ssh-policy
TASK_DIR=$(mktemp -d)
trap 'rm -f "$TASK_DIR/ip" "$TASK_DIR/iptables" "$TASK_DIR/ip6tables" "$TASK_DIR/iptables-restore" "$TASK_DIR/ip6tables-restore" "$TASK_DIR/iptables-restore.rules" "$TASK_DIR/ip6tables-restore.rules" "$TASK_DIR/policy.lock"; rmdir "$TASK_DIR"' EXIT HUP INT TERM
for command in ip iptables ip6tables iptables-restore ip6tables-restore; do
	ln -s "$HERE/test-ssh-policy.sh" "$TASK_DIR/$command"
done
export SSH_POLICY_MOCK=1 SSH_POLICY_CAPTURE=$TASK_DIR
export PATH="$TASK_DIR:$PATH" LAN_IFACE=lan0 WAN_IFACE=wan0
export PHOEBUS_FIREWALL_COMMON=$HERE/../rootfs/etc/s6/scripts/firewall-lock
export PHOEBUS_FIREWALL_LOCK=$TASK_DIR/policy.lock
sh "$POLICY"
for family in 4 6; do
	if [ "$family" = 4 ]; then tool=iptables; prefix=192.0.2.1/24; else tool=ip6tables; prefix=2001:db8:1::1/64; fi
	file=$TASK_DIR/$tool-restore.rules
	grep -Fx -- "-A PHOEBUS_SSH$family -i wan0 -j DROP" "$file"
	grep -Fx -- "-A PHOEBUS_SSH$family -i lan0 -s $prefix -j ACCEPT" "$file"
	grep -Fx -- "-A PHOEBUS_SSH$family -j DROP" "$file"
	grep -Fx -- "-I INPUT 1 -p tcp --dport 22 -j PHOEBUS_SSH$family" "$file"
	[ "$(grep -c '^COMMIT$' "$file")" = 1 ]
done
SSH_POLICY_EXISTING=0 sh "$POLICY"
grep -Fx -- '-D INPUT -p tcp --dport 22 -j PHOEBUS_SSH4' "$TASK_DIR/iptables-restore.rules"
grep -Fx -- '-D INPUT -p tcp --dport 22 -j PHOEBUS_SSH6' "$TASK_DIR/ip6tables-restore.rules"
before=$(wc -l < "$TASK_DIR/iptables-restore.rules")
if SSH_POLICY_LIST_RC=4 sh "$POLICY"; then
	echo 'FAIL: firewall listing failure ignored' >&2
	exit 1
fi
[ "$(wc -l < "$TASK_DIR/iptables-restore.rules")" = "$before" ]
rm -f "$TASK_DIR/policy.lock"
echo 'PASS: first-boot missing chains, listing failures propagate, dual-stack LAN-only SSH, noflush, reload jump replacement'
