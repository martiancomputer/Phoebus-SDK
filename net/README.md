# iptables for Phoebus-OS

Cross-built from the vendor SDK's `user/iptables-1.4.21` (legacy, setsockopt
based -- no libmnl/libnftnl needed):

    ./configure --host=$HOST --prefix=$STAGING \
        --disable-nftables --disable-shared --enable-static --disable-ipv6

Traps hit while doing it:

- Do NOT pass `--with-kernel=<kernel source>`. That drags *internal* kernel
  headers into a userspace build and dies on `asm/rwonce.h`. The toolchain
  sysroot's exported headers are the right ones.
- Three extensions need libraries we do not ship and must be removed from
  `extensions/` before it links: `libxt_connlabel` (libnetfilter_conntrack),
  `libxt_macrange` and `libxt_TCPTERMAC` (vendor extras). None matter for NAT.
- `utils/nfnl_osf` also wants libnfnetlink and aborts the recursive build before
  it reaches the binary. Build `extensions/` and then `iptables/` directly.

Installs as `/sbin/xtables-multi` with `iptables{,-save,-restore}` symlinks.
Needs `/lib/libresolv.so.2` on top of libc/libm.

## Kernel side

Modern kernels split legacy iptables from the nft backend, and the tables now
sit behind the legacy one. `CONFIG_IP_NF_NAT` and `CONFIG_IP_NF_FILTER` are not
even *offered* by Kconfig unless both of these are set:

    CONFIG_NETFILTER_XTABLES_LEGACY=y
    CONFIG_IP_NF_IPTABLES_LEGACY=y

plus `NETFILTER_XT_TARGET_MASQUERADE`, `NETFILTER_XT_MATCH_CONNTRACK`,
`NETFILTER_XT_TARGET_TCPMSS`.
