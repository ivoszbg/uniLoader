#!/usr/bin/env python3
"""Wrap a kernel-like binary (uniLoader) into an Android boot.img, header v4,
no ramdisk (A14 keeps the ramdisk in init_boot/vendor_boot), copying os_version
from your stock boot.img so Android's patch-level checks stay happy.

    python3 pack_boot_v4.py uniLoader --stock boot.img -o boot_uniloader.img
"""
import argparse, struct, sys
ap = argparse.ArgumentParser()
ap.add_argument("kernel"); ap.add_argument("--stock", required=True)
ap.add_argument("-o", "--out", required=True)
a = ap.parse_args()
PAGE, MAXSZ = 4096, 64 * 1024 * 1024
stock = open(a.stock, "rb").read(4096)
if stock[:8] != b"ANDROID!" or struct.unpack_from("<I", stock, 40)[0] != 4:
    sys.exit("--stock must be a header-v4 boot.img")
os_version = struct.unpack_from("<I", stock, 16)[0]
k = open(a.kernel, "rb").read()
hdr = b"ANDROID!" + struct.pack("<IIII", len(k), 0, os_version, 1584)
hdr += b"\0" * 16 + struct.pack("<I", 4) + b"\0" * 1536 + struct.pack("<I", 0)
assert len(hdr) == 1584, len(hdr)
img = hdr.ljust(PAGE, b"\0") + k
img += b"\0" * (-len(img) % PAGE)
if len(img) > MAXSZ: sys.exit(f"image is {len(img)} bytes, boot partition is {MAXSZ}")
open(a.out, "wb").write(img)
print(f"wrote {a.out}: {len(img)} bytes ({len(img)*100//MAXSZ}% of the 64 MiB boot partition)")
