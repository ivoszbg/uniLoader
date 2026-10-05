#!/usr/bin/env python3
"""Print headers of Android boot.img / vendor_boot.img (header v3/v4) and
extract the dtb from vendor_boot.img.  Usage:
    python3 bootimg_info.py boot.img
    python3 bootimg_info.py vendor_boot.img  [-o outdir]
Paste the printed text back (the files themselves are too big to upload)."""
import struct, sys, os

def al(x, p): return (x + p - 1) // p * p
def cstr(b): return b.split(b"\0")[0].decode("latin1")

def boot(d):
    (ks, rs, osv, hs, _r0, _r1, _r2, _r3, ver) = struct.unpack_from("<9I", d, 8)
    cmd = cstr(d[44:44 + 1536])
    print(f"type            : boot.img (header v{ver})")
    print(f"kernel_size     : {ks} ({ks/1048576:.1f} MiB)")
    print(f"ramdisk_size    : {rs}")
    print(f"os_version      : {osv:#x}")
    print(f"header_size     : {hs}")
    print(f"cmdline         : {cmd!r}")
    k = al(4096, 4096)
    print("kernel magic    :", "ARM64 Image" if d[k + 56:k + 60] == b"ARMd" else d[k:k + 8].hex())
    if d[k + 56:k + 60] == b"ARMd":
        text_off, img_size, flags = struct.unpack_from("<QQQ", d, k + 8)
        print(f"Image text_off  : {text_off:#x}   image_size: {img_size:#x}   flags: {flags:#x}")

def vendor(d, out):
    ver, page, kaddr, raddr, vrs = struct.unpack_from("<5I", d, 8)
    cmd = cstr(d[28:28 + 2048]); off = 28 + 2048
    tags = struct.unpack_from("<I", d, off)[0]; name = cstr(d[off + 4:off + 20]); off += 20
    hsz, dtbsz = struct.unpack_from("<2I", d, off); off += 8
    dtbaddr = struct.unpack_from("<Q", d, off)[0]; off += 8
    tsz, tnum, tesz, bcsz = struct.unpack_from("<4I", d, off)
    print(f"type            : vendor_boot.img (header v{ver})")
    print(f"page_size       : {page}")
    print(f"kernel_addr     : {kaddr:#x}")
    print(f"ramdisk_addr    : {raddr:#x}")
    print(f"tags_addr       : {tags:#x}   dtb_addr: {dtbaddr:#x}")
    print(f"vendor ramdisk  : {vrs} bytes, {tnum} fragment(s)")
    print(f"dtb_size        : {dtbsz}   bootconfig_size: {bcsz}")
    print(f"cmdline         : {cmd!r}")
    ro = al(hsz, page); do = ro + al(vrs, page)
    dtb = d[do:do + dtbsz]
    os.makedirs(out, exist_ok=True)
    p = os.path.join(out, "vendor_boot_dtb.img"); open(p, "wb").write(dtb)
    n = 0; i = 0
    while True:
        i = dtb.find(b"\xd0\x0d\xfe\xed", i)
        if i < 0: break
        sz = struct.unpack(">I", dtb[i + 4:i + 8])[0]
        open(os.path.join(out, f"dtb_{n}.dtb"), "wb").write(dtb[i:i + sz]); n += 1; i += sz
    print(f"dtb             : {len(dtb)} bytes -> {p}  ({n} tree(s) split as dtb_N.dtb)")

if __name__ == "__main__":
    a = sys.argv[1:]
    out = a[a.index("-o") + 1] if "-o" in a else "bootimg_out"
    d = open(a[0], "rb").read(64 * 1024 * 1024 + 4096 if False else None)
    if d[:8] == b"ANDROID!": boot(d)
    elif d[:8] == b"VNDRBOOT": vendor(d, out)
    else: sys.exit(f"unknown magic {d[:8]!r}")
