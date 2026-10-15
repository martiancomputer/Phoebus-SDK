#!/usr/bin/env python3
"""Minimal read-only TFTP server for flashing the board from U-Boot.

Replaces dnsmasq --enable-tftp, which silently dropped every read request from
this board: the RRQ arrived intact on the wire (verified by tcpdump), no ICMP
port-unreachable came back so something *was* bound to port 69, and yet dnsmasq
neither answered nor logged. --user=root did not change it, and dnsmasq had not
been upgraded in a month, so the cause was never established. Rather than keep
guessing at another process's silence, this serves the file directly and logs
every packet it sees, so a failure says what happened.

Deliberately not a general TFTP server: read-only, one directory, no writes, no
netascii translation (U-Boot asks for octet).

  RFC 1350  base protocol
  RFC 2347  option negotiation (OACK)
  RFC 2348  blksize      -- U-Boot asks for 1468
  RFC 2349  timeout, tsize

Usage:  tftpd.py <root-dir> [bind-addr] [port]
"""

import os
import stat
import socket
import struct
import sys
import time

OP_RRQ, OP_WRQ, OP_DATA, OP_ACK, OP_ERROR, OP_OACK = 1, 2, 3, 4, 5, 6

ERR_NOT_FOUND = 1
ERR_ACCESS = 2
ERR_ILLEGAL = 4

DEFAULT_BLKSIZE = 512
RETRIES = 6


def log(msg):
    print("%s  %s" % (time.strftime("%H:%M:%S"), msg), flush=True)


def parse_rrq(payload):
    """Split a RRQ body into (filename, mode, options).

    Fields are NUL-terminated strings. A trailing NUL leaves an empty final
    element, so drop empties rather than assuming an exact count -- clients
    differ on whether options are present at all.
    """
    parts = [p.decode("latin-1") for p in payload.split(b"\x00")]
    parts = [p for p in parts if p != ""]
    if len(parts) < 2:
        return None, None, {}
    filename, mode = parts[0], parts[1].lower()
    opts = {}
    rest = parts[2:]
    for i in range(0, len(rest) - 1, 2):
        opts[rest[i].lower()] = rest[i + 1]
    return filename, mode, opts


def send_error(sock, peer, code, msg):
    sock.sendto(struct.pack("!HH", OP_ERROR, code) + msg.encode() + b"\x00", peer)
    log("  -> ERROR %d %s" % (code, msg))


def serve_file(root, peer, filename, opts):
    """Handle one transfer on its own ephemeral socket, as TFTP requires."""
    # The boot server intentionally serves a single directory. Resolve the
    # uImage symlink, then open the resolved direct child relative to root.
    safe = os.path.realpath(os.path.join(root, filename.lstrip("/")))

    xfer = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    xfer.bind(("", 0))

    fh = None
    try:
        if os.path.dirname(safe) != root:
            send_error(xfer, peer, ERR_ACCESS, "Access violation")
            return
        try:
            root_fd = os.open(root, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
            try:
                fd = os.open(os.path.basename(safe),
                             os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK,
                             dir_fd=root_fd)
            finally:
                os.close(root_fd)
            if not stat.S_ISREG(os.fstat(fd).st_mode):
                os.close(fd)
                send_error(xfer, peer, ERR_ACCESS, "Not a regular file")
                return
            try:
                fh = os.fdopen(fd, "rb")
            except OSError:
                os.close(fd)
                raise
        except FileNotFoundError:
            send_error(xfer, peer, ERR_NOT_FOUND, "File not found")
            return
        except OSError as e:
            # The exact failure dnsmasq would not tell us about.
            send_error(xfer, peer, ERR_ACCESS, "Access violation: %s" % e.strerror)
            log("  !! cannot open %s: %s (running as uid %d)" % (safe, e, os.geteuid()))
            return

        size = os.fstat(fh.fileno()).st_size
        blksize = DEFAULT_BLKSIZE
        timeout = 3.0
        ack = {}

        if "blksize" in opts:
            want = int(opts["blksize"])
            blksize = max(8, min(want, 65464))
            ack["blksize"] = str(blksize)
        if "timeout" in opts:
            timeout = float(int(opts["timeout"]))
            if not 1 <= timeout <= 10:
                send_error(xfer, peer, ERR_ILLEGAL, "Unsupported timeout")
                return
            ack["timeout"] = opts["timeout"]
        if "tsize" in opts:
            ack["tsize"] = str(size)

        xfer.settimeout(timeout)
        log("  serving %s (%d bytes) blksize=%d timeout=%gs port=%d"
            % (os.path.basename(safe), size, blksize, timeout, xfer.getsockname()[1]))

        # With options we must OACK first and wait for ACK #0; the data stream
        # only starts once the client has agreed to the block size.
        if ack:
            pkt = struct.pack("!H", OP_OACK)
            for k, v in ack.items():
                pkt += k.encode() + b"\x00" + v.encode() + b"\x00"
            if not send_await(xfer, peer, pkt, 0, "OACK %s" % ack):
                return

        with fh:
            block = 1
            sent = 0
            t0 = time.time()
            while True:
                data = fh.read(blksize)
                pkt = struct.pack("!HH", OP_DATA, block & 0xFFFF) + data
                if not send_await(xfer, peer, pkt, block & 0xFFFF, None):
                    log("  !! transfer aborted at block %d" % block)
                    return
                sent += len(data)
                # A short block ends the transfer; a file that is an exact
                # multiple of blksize still needs that final empty one.
                if len(data) < blksize:
                    dt = time.time() - t0
                    log("  DONE %s -- %d bytes in %.1fs (%.0f KB/s)"
                        % (os.path.basename(safe), sent, dt, sent / 1024.0 / max(dt, 0.001)))
                    return
                block += 1
    finally:
        if fh is not None:
            fh.close()
        xfer.close()


def send_await(sock, peer, pkt, expect_block, what):
    """Send pkt, wait for ACK of expect_block, retransmit on timeout."""
    for attempt in range(RETRIES):
        sock.sendto(pkt, peer)
        if what and attempt == 0:
            log("  -> %s" % what)
        try:
            while True:
                reply, raddr = sock.recvfrom(1024)
                if raddr != peer:
                    continue          # not our client
                if len(reply) < 4:
                    continue
                op, blk = struct.unpack("!HH", reply[:4])
                if op == OP_ERROR:
                    log("  <- ERROR from client: %s" % reply[4:].rstrip(b"\x00").decode("latin-1"))
                    return False
                if op == OP_ACK and blk == expect_block:
                    return True
                # An ACK for an older block is a duplicate; keep waiting.
        except socket.timeout:
            if attempt == 0:
                log("  .. no ACK for block %d, retransmitting" % expect_block)
    return False


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.realpath(sys.argv[1])
    bind = sys.argv[2] if len(sys.argv) > 2 else "0.0.0.0"
    port = int(sys.argv[3]) if len(sys.argv) > 3 else 69

    if not os.path.isdir(root):
        print("no such directory: %s" % root, file=sys.stderr)
        return 1

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind((bind, port))
    except OSError as e:
        print("cannot bind %s:%d -- %s" % (bind, port, e), file=sys.stderr)
        return 1

    log("tftpd listening on %s:%d, root=%s, uid=%d" % (bind, port, root, os.geteuid()))

    while True:
        try:
            data, peer = sock.recvfrom(2048)
        except KeyboardInterrupt:
            log("stopped")
            return 0
        if len(data) < 2:
            continue
        op = struct.unpack("!H", data[:2])[0]
        if op == OP_WRQ:
            log("%s:%d WRQ refused (read-only server)" % peer)
            send_error(sock, peer, ERR_ILLEGAL, "Server is read-only")
            continue
        if op != OP_RRQ:
            log("%s:%d ignoring opcode %d" % (peer[0], peer[1], op))
            continue

        filename, mode, opts = parse_rrq(data[2:])
        if filename is None:
            send_error(sock, peer, ERR_ILLEGAL, "Malformed request")
            continue
        log("%s:%d RRQ %r mode=%s opts=%s" % (peer[0], peer[1], filename, mode, opts or "{}"))
        try:
            serve_file(root, peer, filename, opts)
        except Exception as e:                       # never let one client kill the server
            log("  !! %s: %r" % (type(e).__name__, e))


if __name__ == "__main__":
    sys.exit(main())
