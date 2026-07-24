#!/bin/sh
#
# Cross-compile the skarnet s6 supervision suite, fully static, for a target
# triple. Produces $STAGING/bin/* (static, stripped) -- the rootfs build then
# copies a curated subset in.
#
# Usage:
#   HOST=mips-buildroot-linux-gnu \
#   PATH=/path/to/toolchain/bin:$PATH \
#   ./build.sh
#
# Optional env: CC, STRIP (default $HOST-gcc / $HOST-strip), STAGING, SRC.
# --------------------------------------------------------------------------
set -e

HERE=$(cd "$(dirname "$0")" && pwd)
: "${HOST:?set HOST to the cross triple, e.g. mips-buildroot-linux-gnu}"
: "${SRC:=$HERE/src}"
: "${STAGING:=$HERE/staging}"
export CC="${CC:-$HOST-gcc}"
# MIPS32 glibc needs BOTH large-file and 64-bit-time support, or skalibs hits
# EOVERFLOW at boot:
#   _FILE_OFFSET_BITS=64 -> 64-bit stat (large inodes/sizes)
#   _TIME_BITS=64        -> 64-bit time_t, so skalibs' SKALIBS_SIZEOFTIME==8 and
#                          its "value too large" guard on tain->sysclock deadline
#                          conversion (time_sysclock_from_tai) compiles out.
# glibc requires _FILE_OFFSET_BITS=64 whenever _TIME_BITS=64.
export CFLAGS="${CFLAGS:--O2 -pipe -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64}"
STRIP="${STRIP:-$HOST-strip}"

# Pinned, mutually-compatible skarnet releases.
SKALIBS=v2.15.1.0
EXECLINE=v2.9.9.2
S6=v2.15.1.0
S6_RC=v0.7.0.0
S6_LINUX_INIT=v1.2.0.2

mkdir -p "$SRC"
rm -rf "$STAGING"; mkdir -p "$STAGING"

fetch() {  # fetch <repo> <tag>
	[ -d "$SRC/$1/.git" ] && return 0
	git clone --quiet --depth 1 --branch "$2" "https://github.com/skarnet/$1" "$SRC/$1"
}

# skalibs cross sysdeps that cannot be autodetected (modern glibc Linux target).
SKA_SYSDEPS="--with-sysdep-devurandom=yes --with-sysdep-posixspawnearlyreturn=no \
--with-sysdep-selectinfinite=yes --with-sysdep-procselfexe=/proc/self/exe"

# Where downstream packages find the already-built skalibs.
WITH="--with-include=$STAGING/include --with-lib=$STAGING/lib \
--with-dynlib=$STAGING/lib --with-sysdeps=$STAGING/lib/skalibs/sysdeps"

build() {  # build <pkg> <configure-args...>
	pkg=$1; shift
	cd "$SRC/$pkg"
	echo "==== $pkg ===="
	make distclean >/dev/null 2>&1 || true
	# --enable-allstatic  => link skarnet libs (skalibs/execline/s6) statically into
	#   each binary, but leave libc DYNAMIC (shared glibc runtime lives in /lib).
	#   This keeps the full suite ~7M instead of ~89M fully-static, and scales as
	#   the service tree grows. (Fully-static needs --enable-static-libc.)
	./configure --host="$HOST" --prefix=/ --enable-static --disable-shared \
		--enable-allstatic "$@"
	make -j"$(nproc)"
	make install DESTDIR="$STAGING"
}

fetch skalibs        "$SKALIBS"
fetch execline       "$EXECLINE"
fetch s6             "$S6"
fetch s6-rc          "$S6_RC"
fetch s6-linux-init  "$S6_LINUX_INIT"

build skalibs        $SKA_SYSDEPS
build execline       $WITH
build s6             $WITH
build s6-rc          $WITH
build s6-linux-init  $WITH

# strip everything we staged
find "$STAGING" -type f -perm -u+x -exec sh -c '
  for f; do head -c4 "$f" | grep -q ELF && "'"$STRIP"'" "$f" 2>/dev/null; done' _ {} +

echo "s6 stack built: $(ls "$STAGING/bin" | wc -l) binaries in $STAGING/bin"
