#!/usr/bin/env python3
"""Exercise FC policy transitions in a fake filesystem, never on a router."""
import concurrent.futures
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest


def mock_tool(name):
    root = Path(os.environ["FC_TEST_ROOT"])
    proc = root / "proc"
    with (root / "events").open("a") as stream:
        stream.write(name + " " + " ".join(sys.argv[1:]) + "\n")
    if name == "awk":
        # Emulate the kernel proc getter after its setter has consumed input.
        # Test the real text parser separately below with the host's awk.
        if (proc / "ctrl/hwnat").read_text().strip() != "4":
            return 9
        text = (proc / "ctrl/wan_port_mask").read_text().strip()
        if os.environ.get("FC_TEST_BAD_READBACK"):
            print("0x20")
        elif len(text.split()) == 2 and text.split()[1] == "0":
            print(text.split()[0])
        else:
            return 8
    elif name == "iptables":
        if sys.argv[1:3] != ["-w", "10"]:
            return 7
        if os.environ.get("FC_TEST_RULE_FAIL") and "-I" in sys.argv:
            return 4
    elif name in ("modprobe", "insmod"):
        (root / "modules").write_text("rtk_fc 1 0 - Live 0\n")
        (proc / "ctrl/hwnat").write_text("4\n")
    elif name == "rmmod":
        (root / "modules").write_text("")
    return 0


if Path(sys.argv[0]).name in {"awk", "iptables", "modprobe", "insmod", "rmmod"}:
    sys.exit(mock_tool(Path(sys.argv[0]).name))

SDK = Path(__file__).resolve().parents[1]
POLICY = SDK / "rootfs/etc/s6/scripts/fc-policy"
CONTROL = SDK / "rootfs/usr/sbin/rtk-fc"
FIREWALL = SDK / "rootfs/etc/s6/scripts/firewall-lock"


class WanPolicy(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="fc-wan-test-")
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for directory in ("proc/ctrl", "proc/sw_dump", "net/eth0.8", "bin"):
            (self.root / directory).mkdir(parents=True)
        self.write("proc/ctrl/hwnat", "1\n")
        self.write("proc/ctrl/wan_port_mask", "Dump WAN port mask:0x20\n")
        self.write("proc/ctrl/skbmark_skipFcFunc", "")
        self.write("proc/sw_dump/ext_flowmib", "0\n")
        self.write("net/eth0.8/rtk_tx_port_mask", "0x40\n")
        self.write("wan", "eth0.8\n")
        self.write("cmdline", "console=ttyS0 wan=eth0.8\n")
        self.write("modules", "rtk_fc 1 0 - Live 0\n")
        for command in ("awk", "iptables", "modprobe", "insmod", "rmmod"):
            (self.root / "bin" / command).symlink_to(Path(__file__).resolve())
        self.env = {
            **os.environ,
            "PATH": str(self.root / "bin") + os.pathsep + os.environ["PATH"],
            "FC_TEST_ROOT": str(self.root),
            "PHOEBUS_FC_POLICY": str(POLICY),
            "PHOEBUS_FC_PROC": str(self.root / "proc"),
            "PHOEBUS_FC_NET": str(self.root / "net"),
            "PHOEBUS_FC_WAN_FILE": str(self.root / "wan"),
            "PHOEBUS_FC_CMDLINE": str(self.root / "cmdline"),
            "PHOEBUS_FC_MODULES": str(self.root / "modules"),
            "PHOEBUS_FC_LOCK": str(self.root / "fc.lock"),
            "PHOEBUS_FIREWALL_COMMON": str(FIREWALL),
            "PHOEBUS_FIREWALL_LOCK": str(self.root / "firewall.lock"),
        }
        self.env.pop("PHOEBUS_FIREWALL_LOCKED", None)

    def write(self, name, text):
        (self.root / name).write_text(text)

    def run_control(self, *args, **env):
        return subprocess.run(["sh", str(CONTROL), *args], env={**self.env, **env},
                              capture_output=True, text=True, timeout=8)

    def assert_cpu_only(self, result):
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual((self.root / "proc/ctrl/hwnat").read_text().strip(), "4")

    def test_boot_load_repairs_pon_mask_before_enabling(self):
        result = self.run_control("load", "1")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / "proc/ctrl/wan_port_mask").read_text(), "0x40 0\n")
        self.assertEqual((self.root / "proc/ctrl/hwnat").read_text(), "1\n")
        self.assertEqual((self.root / "proc/ctrl/skbmark_skipFcFunc").read_text(), "mark1 27\n")
        events = (self.root / "events").read_text().splitlines()
        self.assertTrue(events[0].startswith("awk "))
        self.assertEqual(sum(" -I " in e for e in events), 2)

    def test_default_load_enables_hardware_after_wan_verification(self):
        result = self.run_control("load")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / "proc/ctrl/hwnat").read_text(), "1\n")
        self.assertEqual((self.root / "proc/ctrl/wan_port_mask").read_text(), "0x40 0\n")

    def test_reload_reapplies_mapping(self):
        self.write("modules", "")
        result = self.run_control("load", "2")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("modprobe rtk_fc", (self.root / "events").read_text())
        self.assertEqual((self.root / "proc/ctrl/hwnat").read_text(), "2\n")

    def test_mode_enable_verifies_topology(self):
        result = self.run_control("mode", "3")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / "proc/ctrl/wan_port_mask").read_text(), "0x40 0\n")
        self.assertEqual((self.root / "proc/ctrl/hwnat").read_text(), "3\n")

    def test_existing_mask_still_reapplies_acl_and_clears_wifi_wan(self):
        self.write("proc/ctrl/wan_port_mask",
                   "Dump WAN port mask:0x40\nDump WAN port - wlan dev mask:\n0x1\n")
        result = self.run_control("mode", "1")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / "proc/ctrl/wan_port_mask").read_text(), "0x40 0\n")

    def test_interface_names_do_not_determine_mask(self):
        (self.root / "net/uplink0").mkdir()
        self.write("net/uplink0/rtk_tx_port_mask", "0x100\n")
        self.write("wan", "uplink0\n")
        result = self.run_control("load", "1")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / "proc/ctrl/wan_port_mask").read_text(), "0x100 0\n")

    def test_bad_masks_never_enable_learning(self):
        for mask in ("0x0", "0x60", "0x800", "0x", "64", "0x-1", "0x40;false",
                     "0x100000000", "0x040\n0x020"):
            with self.subTest(mask=mask):
                self.write("proc/ctrl/hwnat", "1\n")
                self.write("net/eth0.8/rtk_tx_port_mask", mask + "\n")
                self.assert_cpu_only(self.run_control("mode", "1"))

    def test_missing_driver_metadata_fails_safely(self):
        (self.root / "net/eth0.8/rtk_tx_port_mask").unlink()
        self.assert_cpu_only(self.run_control("load", "2"))

    def test_readback_mismatch_and_rule_failure_leave_cpu_mode(self):
        self.assert_cpu_only(self.run_control("load", "1", FC_TEST_BAD_READBACK="1"))
        self.assert_cpu_only(self.run_control("load", "1", FC_TEST_RULE_FAIL="1"))

    def test_rejected_proc_write_leaves_cpu_mode(self):
        target = self.root / "proc/ctrl/wan_port_mask"
        target.unlink()
        target.symlink_to("/dev/full")
        self.assert_cpu_only(self.run_control("mode", "1"))

    def test_disabling_needs_no_topology(self):
        (self.root / "net/eth0.8/rtk_tx_port_mask").unlink()
        for mode in ("0", "4"):
            result = self.run_control("mode", mode)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_cmdline_fallback_and_invalid_iface(self):
        (self.root / "wan").unlink()
        self.assertEqual(self.run_control("load", "2").returncode, 0)
        self.write("wan", "../bad\n")
        self.assert_cpu_only(self.run_control("mode", "1"))

    def test_concurrent_transitions_are_serialized(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
            results = list(pool.map(lambda m: self.run_control("mode", m), ("1", "2")))
        for result in results:
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_real_proc_parser_rejects_stale_wifi_mask(self):
        env = {**self.env, "PATH": os.environ["PATH"]}
        for text, success in (
            ("Dump WAN port mask:0x40\n", True),
            ("Dump WAN port mask:0x40\nDump WAN port - wlan dev mask:\n0x1\n", False),
            ("Dump WAN port mask:0x40\nDump WAN port - wlan dev mask:\n", False),
            ("garbage\n", False),
        ):
            self.write("proc/ctrl/wan_port_mask", text)
            result = subprocess.run(["sh", "-c", '. "$PHOEBUS_FC_POLICY"; fc_read_wan_mask'],
                                    env=env, capture_output=True, text=True, timeout=4)
            self.assertEqual(result.returncode == 0, success, result.stdout)

    def test_startup_selects_wan_before_fc_and_preserves_cpu_fallback(self):
        startup = (SDK / "rootfs/etc/s6/scripts/network-up").read_text()
        self.assertLess(startup.index('> /run/wan_iface'), startup.index('rtk-fc load "$HWNAT"'))
        self.assertIn('if rtk-fc load "$HWNAT"; then', startup)
        driver = (SDK / "vendor/realtek-net/rtl86900/FleetConntrackDriver/core/rtk_fc_driver.c").read_text()
        self.assertIn('fc_db.controlFuc.hwnat_mode = RT_FLOW_HWNAT_MODE_TRAP_ALL;', driver)
        nic = (SDK / "vendor/realtek-net/rtl86900/nicDriver/re8686_rtl9607c.c").read_text()
        self.assertIn('READ_ONCE(DEVPRIV(dev)->txPortMask)', nic)
        self.assertEqual(nic.count('sysfs_groups[0] = &rtk_port_group'), 2)

    def test_driver_direction_and_wan_error_propagation(self):
        # Compile the actual bounded functions with hardware calls stubbed out.
        # This checks their behavior, not a separately rewritten Python model.
        core = SDK / "vendor/realtek-net/rtl86900/FleetConntrackDriver/core"

        def body(path, name):
            source = path.read_text()
            definition = re.search(r"^[a-zA-Z_][^\n;{}]*\b" + re.escape(name) + r"\(",
                                   source, re.MULTILINE)
            self.assertIsNotNone(definition, name)
            start = source.index("{", definition.start())
            depth = 0
            for offset in range(start, len(source)):
                depth += (source[offset] == "{") - (source[offset] == "}")
                if depth == 0:
                    return source[start:offset + 1]
            raise AssertionError("unclosed function")

        text = r'''
#define _GNU_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
typedef int rtk_fc_ret_t;
typedef unsigned long long rtk_fc_port_mask_t;
typedef int rtk_fc_flow_direction_t;
typedef unsigned char uint8;
#define TRUE 1
#define FALSE 0
#define RTK_FC_RET_OK 0
#define RTK_FC_MAC_PORT_PON 5
#define RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU 0x1ff
#define RTK_FC_ACLANDCF_RESERVED_TAIL_END 123
#define RTK_FC_FLOW_DIRECTION_UPSTREAM 1
#define RTK_FC_FLOW_DIRECTION_DOWNSTREAM 2
#define RTK_FC_FLOW_DIRECTION_WAN_TO_WAN 3
#define RTK_FC_FLOW_DIRECTION_LANBRIDGE 4
#define RTK_FC_FLOW_DIRECTION_NA 255
#define DEBUG(...) ((void)0)
#define CMD_BUFF_SIZE 256
struct file;
struct { struct { unsigned long long portmask; } wanPortMask, lanPortMask;
         unsigned long long wanPortWifidevMask; } fc_db;
static int acl_result;
static unsigned long long manager_mask;
#define RTK_FC_HELPER_MGR_WAN_PORT_MASK_SET(v) (manager_mask = (v))
static int _rtk_rg_aclAndCfReservedRuleAdd(int type, void *p) { return acl_result; }
static size_t strscpy(char *d, const char *s, size_t n) {
    if (!n) return 0;
    size_t l = strnlen(s, n - 1); memcpy(d, s, l); d[l] = 0; return l;
}
#define simple_strtol(s,e,b) strtol((s),(e),(b))
#define simple_strtoll(s,e,b) strtoll((s),(e),(b))
#include <stdlib.h>
typedef struct { struct { unsigned macPortIdx; } egressPort;
    unsigned egrWlanDevIdx, igrWlanDevIdx; int direction; } rtk_fc_pktHdr_t;
typedef struct { unsigned ingressPort; } rtk_fc_ingress_data_t;
'''
        text += "rtk_fc_ret_t rtk_fc_wan_portmask_set(rtk_fc_port_mask_t wanpmsk)\n"
        text += body(core / "rtk_fc_internal.c", "rtk_fc_wan_portmask_set")
        text += "\nint rtk_fc_proc_wanPortMask_set(struct file *filp, const char *buff, unsigned long len, void *data)\n"
        text += body(core / "rtk_fc_proc.c", "rtk_fc_proc_wanPortMask_set")
        text += "\nrtk_fc_ret_t _rtk_fc_flow_direction(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *ingress_data)\n"
        text += body(core / "rtk_fc_internal.c", "_rtk_fc_flow_direction")
        text += r'''
int main(void) {
    rtk_fc_pktHdr_t p = {.egressPort.macPortIdx = 6};
    rtk_fc_ingress_data_t in = {.ingressPort = 9};
    fc_db.wanPortMask.portmask = 0x20;
    _rtk_fc_flow_direction(&p, &in);
    assert(p.direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE);
    assert(rtk_fc_proc_wanPortMask_set(NULL, "0x40 0\n", 7, NULL) == 7);
    assert(manager_mask == 0x40 && fc_db.wanPortMask.portmask == 0x40);
    _rtk_fc_flow_direction(&p, &in);
    assert(p.direction == RTK_FC_FLOW_DIRECTION_UPSTREAM);
    in.ingressPort = 6; p.egressPort.macPortIdx = 9;
    _rtk_fc_flow_direction(&p, &in);
    assert(p.direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM);
    in.ingressPort = 0;
    _rtk_fc_flow_direction(&p, &in);
    assert(p.direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE);
    in.ingressPort = 6; p.egressPort.macPortIdx = 6;
    _rtk_fc_flow_direction(&p, &in);
    assert(p.direction == RTK_FC_FLOW_DIRECTION_WAN_TO_WAN);
    acl_result = 1234;
    assert(rtk_fc_wan_portmask_set(0x40) == acl_result);
    assert(rtk_fc_proc_wanPortMask_set(NULL, "0x40 0\n", 7, NULL) == -EIO);
    return 0;
}
'''
        source = self.root / "driver-test.c"
        source.write_text(text)
        executable = self.root / "driver-test"
        compile_result = subprocess.run(["cc", "-std=gnu11", "-Werror=implicit-function-declaration",
                                         str(source), "-o", str(executable)],
                                        capture_output=True, text=True, timeout=20)
        self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
        subprocess.run([str(executable)], check=True, timeout=4)


if __name__ == "__main__":
    unittest.main()
