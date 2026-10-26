#!/usr/bin/env python3
"""Host-only fake sysfs/hostapd tests; never accesses router hardware."""
import array
import errno
import os
from pathlib import Path
import random
import signal
import socket
import statistics
import struct
import subprocess
import tempfile
import threading
import time

SRC = Path(__file__).resolve().parents[1]
MSG = struct.Struct("=IHHIIIi10I")
MAGIC = 0x47504931


def packet(op=1, target=0, arg=0, **kw):
    return MSG.pack(kw.get("magic", MAGIC), kw.get("version", 1), op,
                    target, arg, kw.get("flags", 0), 0, *([0]*10))


def wait_for(predicate, timeout=3):
    end = time.monotonic()+timeout
    while time.monotonic() < end:
        if predicate():
            return
        time.sleep(0.005)
    raise AssertionError("event deadline exceeded")


class AP:
    def __init__(self, path):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        self.sock.bind(str(path))
        self.sock.settimeout(.1)
        self.peer = None
        self.running = True
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        while self.running:
            try:
                data, peer = self.sock.recvfrom(4096)
                self.peer = peer
                if data == b"ATTACH":
                    self.sock.sendto(b"OK\n", peer)
                elif data == b"STATUS":
                    self.sock.sendto(b"state=ENABLED\n", peer)
            except socket.timeout:
                pass
            except OSError:
                if self.running:
                    raise

    def event(self, value):
        self.sock.sendto(b"<3>"+value.encode(), self.peer)

    def close(self):
        self.running = False
        self.thread.join(1)
        self.sock.close()


with tempfile.TemporaryDirectory(prefix="gpio-") as temp:
    root = Path(temp)
    common = ["cc", "-std=c11", "-D_GNU_SOURCE", "-DGPIO_TEST", "-Wall", "-Wextra", "-Werror", "-O2"]
    unit = [str(SRC/"tests/unit.c"), str(SRC/"config.c")]
    subprocess.run(common+unit+["-o", str(root/"unit")], check=True)
    subprocess.run([str(root/"unit")], check=True)
    subprocess.run(common+["-DGPIO_SANITIZER_TEST", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]+
                   unit+["-o", str(root/"unit-asan")], check=True)
    subprocess.run([str(root/"unit-asan")], check=True)
    subprocess.run(common+[str(SRC/"daemon.c"), str(SRC/"config.c"), "-o", str(root/"daemon")], check=True)
    subprocess.run(common+[str(SRC/"gpioctl.c"), "-o", str(root/"gpioctl")], check=True)
    for directory in ("etc/phoebus", "run/phoebus", "run/hostapd"):
        (root/directory).mkdir(parents=True, exist_ok=True)
    conf = root/"etc/phoebus/gpio.conf"
    defaults = (SRC.parent/"rootfs/etc/phoebus/gpio.conf").read_text()
    conf.write_text(defaults)
    conf.chmod(0o640)
    for name in ("power", "lan", "internetgreen", "internetorange", "wlan2g", "wlan5g", "wps"):
        led = root/"sys/class/leds"/name
        led.mkdir(parents=True)
        (led/"brightness").write_text("0\n")
        (led/"trigger").write_text("none\n")

    def brightness(name):
        return int((root/"sys/class/leds"/name/"brightness").read_text().splitlines()[0])

    aps = [AP(root/"run/hostapd"/f"wlan{r}") for r in range(2)]
    log = (root/"stderr").open("w+")
    daemon = subprocess.Popen([str(root/"daemon"), "--test-root", str(root)], stderr=log)
    sockpath = str(root/"run/phoebus/gpio.sock")

    def request(data=packet(), ancillary=None):
        with socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET) as s:
            s.settimeout(1)
            s.connect(sockpath)
            if ancillary:
                s.sendmsg([data], ancillary)
            else:
                s.send(data)
            reply = s.recv(512)
            assert len(reply) == MSG.size
            return MSG.unpack(reply)

    try:
        wait_for(lambda: Path(sockpath).exists() or daemon.poll() is not None)
        assert daemon.poll() is None
        wait_for(lambda: brightness("wlan2g") == brightness("wlan5g") == 255)
        time.sleep(.1)
        assert brightness("power") == 255
        assert brightness("lan") == brightness("internetgreen") == brightness("internetorange") == 0
        assert request()[6] == 0
        if os.getuid() != 0:
            assert request(packet(5, 5, 2))[6] == -errno.EPERM
            assert request(packet(3))[6] == -errno.EPERM
        for data, expected in [(packet(version=99), errno.EINVAL), (packet(flags=1), errno.EINVAL),
                               (b"x"*63, errno.EMSGSIZE), (b"x"*65, errno.EMSGSIZE),
                               (b"x"*8192, errno.EMSGSIZE), (packet(65535), errno.EINVAL),
                               (packet(2, 99, 500), errno.EINVAL)]:
            assert request(data)[6] == -expected
        with (root/"stderr").open("rb") as passed:
            assert request(packet(), [(socket.SOL_SOCKET, socket.SCM_RIGHTS,
                                       array.array("i", [passed.fileno()]))])[6] == -errno.EMSGSIZE
        assert request(packet(2, 1, 100))[6] == 0
        assert brightness("lan") == 255
        wait_for(lambda: brightness("lan") == 0)
        conf.write_text(defaults.replace("brightness = 255", "brightness = 123"))
        os.kill(daemon.pid, signal.SIGHUP)
        wait_for(lambda: brightness("power") == 123)
        conf.write_text("[general]\nbrightness=999\n")
        os.kill(daemon.pid, signal.SIGHUP)
        time.sleep(.05)
        assert brightness("power") == 123
        conf.unlink()
        conf.symlink_to(root/"stderr")
        os.kill(daemon.pid, signal.SIGHUP)
        time.sleep(.05)
        assert brightness("power") == 123
        conf.unlink()
        conf.write_text(defaults)
        conf.chmod(0o640)
        os.kill(daemon.pid, signal.SIGHUP)
        wait_for(lambda: brightness("power") == 255)
        samples = []
        for _ in range(12):
            aps[1].event("AP-DISABLED")
            wait_for(lambda: brightness("wlan2g") == 0)
            start = time.perf_counter()
            aps[1].event("AP-ENABLED")
            wait_for(lambda: brightness("wlan2g") == 255)
            samples.append((time.perf_counter()-start)*1000)

        def cpu_ticks():
            fields = Path(f"/proc/{daemon.pid}/stat").read_text().split()
            return int(fields[13])+int(fields[14])

        time.sleep(.1)
        before = cpu_ticks()
        time.sleep(2)
        idle_ticks = cpu_ticks()-before
        status = Path(f"/proc/{daemon.pid}/status").read_text()
        assert "NoNewPrivs:\t1" in status and "Seccomp:\t2" in status
        assert idle_ticks <= 1, f"unexpected idle workload: {idle_ticks} ticks"
        rss = next(x for x in status.splitlines() if x.startswith("VmRSS:"))
        print(f"HOST idle CPU: {idle_ticks} ticks/2s; {rss}; median event latency {statistics.median(samples):.3f} ms")
        aps[0].event("AP-DISABLED")
        aps[1].event("AP-DISABLED")
        conf.write_text(defaults.replace("on_system_ready = solid", "on_system_ready = blink"))
        os.kill(daemon.pid, signal.SIGHUP)
        time.sleep(.05)
        before = cpu_ticks()
        time.sleep(2)
        print(f"HOST simultaneous power + both radio animation CPU: {cpu_ticks()-before} ticks/2s")
        children = Path(f"/proc/{daemon.pid}/task/{daemon.pid}/children").read_text().strip()
        assert not children, "daemon must never spawn workers"
        conf.write_text(defaults)
        os.kill(daemon.pid, signal.SIGHUP)
        aps[0].event("AP-ENABLED")
        aps[1].event("AP-ENABLED")
        rng = random.Random(1)
        for _ in range(200):
            request(rng.randbytes(rng.randint(1, 256)))
        assert daemon.poll() is None
        aps[1].event("AP-DISABLED")
        wait_for(lambda: brightness("wlan2g") == 0)
        daemon.kill()
        assert daemon.wait(2) == -signal.SIGKILL
        daemon = subprocess.Popen([str(root/"daemon"), "--test-root", str(root)], stderr=log)
        time.sleep(.05)
        wait_for(lambda: Path(sockpath).exists() or daemon.poll() is not None)
        wait_for(lambda: brightness("wlan2g") == brightness("wlan5g") == 255)
        assert daemon.poll() is None
        assert request()[13:15] == (1, 1), "restart must reconstruct real hostapd readiness"
        print("PASS: IPC framing/auth, fd rejection, atomic reload/symlink safety, hostapd events, idle CPU, restart")
    finally:
        if daemon.poll() is None:
            daemon.terminate()
            try:
                daemon.wait(2)
            except subprocess.TimeoutExpired:
                daemon.kill()
                daemon.wait()
        for ap in aps:
            ap.close()
        log.seek(0)
        print(log.read())
        log.close()
