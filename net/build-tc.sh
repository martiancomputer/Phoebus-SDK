#!/bin/sh
# Cross-build iproute2's `tc` for Phoebus boards.
#
# Why this exists at all: nothing in the tree ships a tc. busybox's applet is
# disabled (and could not drive cake anyway), and the only iproute2 in the
# vendor dumps is 4.4.0 -- CAKE support landed in iproute2 4.19, so those
# sources cannot configure it even once built. Without a modern tc the kernel's
# CONFIG_NET_SCH_CAKE=y is unreachable.
#
#   HOST=mips-buildroot-linux-gnu PATH=/path/to/toolchain/bin:$PATH ./build-tc.sh
#
# Produces $STAGING/sbin/tc. Only tc is installed -- ip/ss/bridge would collide
# with the busybox applets the rootfs already uses.
set -e

HERE=$(cd "$(dirname "$0")" && pwd)
: "${HOST:?set HOST to the cross triple, e.g. mips-buildroot-linux-gnu}"
: "${IPR_VER:=6.11.0}"
: "${SRC:=$HERE/.work}"
: "${STAGING:=$HERE/staging}"
CC="${CC:-$HOST-gcc}"

mkdir -p "$SRC" "$STAGING/sbin"
cd "$SRC"

TARBALL="iproute2-$IPR_VER.tar.xz"
if [ ! -d "iproute2-$IPR_VER" ]; then
	[ -f "$TARBALL" ] || curl -fL --retry 3 -O \
		"https://www.kernel.org/pub/linux/utils/net/iproute2/$TARBALL"
	tar xf "$TARBALL"
fi
cd "iproute2-$IPR_VER"

# configure is a hand-rolled script, not autoconf: it probes by compiling test
# programs with $CC, so cross-detection works as long as CC points at the cross
# compiler. It cannot be given --host.
#
# The optional libraries are all deliberately off:
#   libelf   only needed for `tc` BPF object loading, which we do not use
#   libmnl   only used by ip/bridge paths we do not install
#   libcap   selinux/capability plumbing, irrelevant here
# Leaving them on makes configure emit a build that fails to link against a
# sysroot that has none of them.
make clean >/dev/null 2>&1 || true
CC="$CC" ./configure --libbpf_force off >/dev/null

# Disable every optional library configure "found".
#
# configure probes with $CC but several checks fall through to pkg-config or
# bare /usr paths, which resolve against the BUILDER's filesystem -- so on a
# normal Linux host it happily reports HAVE_ELF, HAVE_MNL, HAVE_CAP and
# HAVE_RPC and writes `-I/usr/include/tirpc` into config.mk. The cross gcc then
# refuses with "unsafe header/library path used in cross-compilation", which is
# correct: those are x86 headers.
#
# None of them are needed for tc+cake. libelf is only for BPF object loading,
# libmnl and libcap for ip/bridge paths we do not install. Turning them off is
# both the fix and the smaller binary.
sed -i -e '/^HAVE_ELF:=/d'  -e '/-DHAVE_ELF/d' \
       -e '/^HAVE_MNL:=/d'  -e '/-DHAVE_LIBMNL/d' \
       -e '/^HAVE_CAP:=/d'  -e '/-DHAVE_LIBCAP/d' \
       -e '/^HAVE_RPC:=/d'  -e '/-DHAVE_RPC/d' \
       -e 's|-I/usr/include[^ ]*||g' -e 's|-L/usr/lib[^ ]*||g' -e 's|-ltirpc||g' \
       config.mk

# The -D defines and the link flags are separate lines; dropping only the
# former still leaves `LDLIBS += -lelf/-lmnl/-lcap` and the link fails with
# "cannot find -lelf" against a sysroot that has none of them.
sed -i -e 's|^LDLIBS += *-lelf$|LDLIBS +=|' \
       -e 's|^LDLIBS += *-lmnl$|LDLIBS +=|' \
       -e 's|^LDLIBS += *-lcap$|LDLIBS +=|' \
       config.mk

# Build from the TOP level with SUBDIRS narrowed, not `make -C lib`.
#
# lib/Makefile inherits its include path from the top-level Makefile, so
# entering the subdirectory directly builds without -I../include and dies on
# "libgenl.h: No such file or directory". Narrowing SUBDIRS keeps the correct
# flags while skipping ip/, bridge/, rdma/ and the rest -- several of which
# still want libmnl and would abort the run before tc is reached.
make CC="$CC" SUBDIRS="lib tc"

"$HOST-strip" tc/tc
cp tc/tc "$STAGING/sbin/tc"

# Prove the binary can actually do the one thing it was built for. A tc without
# cake compiled in fails at runtime with "Unknown qdisc", which on a router
# reads as a kernel problem rather than a userspace one.
if ! strings "$STAGING/sbin/tc" | grep -q '^cake$'; then
	echo "build-tc: FAILED -- no cake support in the resulting binary" >&2
	exit 1
fi

echo "tc built: $STAGING/sbin/tc ($(stat -c %s "$STAGING/sbin/tc") bytes, cake OK)"
