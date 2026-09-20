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

## Dropbear SSH

The RAM-boot image currently uses upstream Dropbear **2026.94**, downloaded
from the author's release directory:

    https://matt.ucc.asn.au/dropbear/releases/dropbear-2026.94.tar.bz2

Tarball SHA-256:
`e098034a843699200c8c977a991fff73159735bf795d5f72ef672c41a6b1ae81`.
Both the detached tarball signature and upstream `SHA256SUM.asc` were checked
with the Dropbear release-key fingerprint
`F734 7EF2 EE2E 07A2 6762 8CA9 4493 1494 F29C 6773`.

Configure with the SDK's absolute cross-toolchain path first in `PATH` and
`$STAGING` set to `net-build/staging`:

    ./configure --host=mips-buildroot-linux-gnu \
        --disable-zlib --disable-lastlog --disable-utmp --disable-utmpx \
        --disable-wtmp --disable-wtmpx --disable-pututline \
        --disable-pututxline --enable-bundled-libtom \
        CFLAGS="-Os -fPIE -fstack-protector-strong -D_FORTIFY_SOURCE=2 -I$STAGING/include" \
        LDFLAGS="-Wl,-z,relro,-z,now -pie -L$STAGING/lib" LIBS=-lcrypt
    make -j8 PROGRAMS='dropbear dbclient dropbearkey scp' MULTI=1 SCPPROGRESS=1
    mips-buildroot-linux-gnu-strip --strip-unneeded dropbearmulti

Install the stripped multicall executable as `/usr/sbin/dropbearmulti`, with
`dropbear`, `dbclient`, `dropbearkey`, and `scp` symlinked to it. The verified
binary in image #166 has SHA-256
`f1ab4fc810c5bd0110c4f81da73fdd33e969bb94a284fcf57eb60badc7601da3`.
It is MIPS32 big-endian PIE with GNU RELRO, immediate binding and stack-canary
references. Upstream defaults leave X11, CBC and DSS disabled while enabling
Ed25519 and the SNTRUP761/ML-KEM768 hybrid exchanges. The s6 service retains
the already-tested ECDSA P-256 host-key policy to isolate the daemon upgrade.

## Kernel side

Modern kernels split legacy iptables from the nft backend, and the tables now
sit behind the legacy one. `CONFIG_IP_NF_NAT` and `CONFIG_IP_NF_FILTER` are not
even *offered* by Kconfig unless both of these are set:

    CONFIG_NETFILTER_XTABLES_LEGACY=y
    CONFIG_IP_NF_IPTABLES_LEGACY=y

plus `NETFILTER_XT_TARGET_MASQUERADE`, `NETFILTER_XT_MATCH_CONNTRACK`,
`NETFILTER_XT_TARGET_TCPMSS`.
