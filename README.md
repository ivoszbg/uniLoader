# uniLoader

**uniLoader** is a minimalistic loader, capable of booting Linux kernels.
It can be used as an intermediate bootloader, providing a clean booting
environment in case of a forced and buggy bootloader.

---

## Supported Architectures
- ARMv7
- ARMv8

---

## Make Syntax
```bash
make ARCH=$(arch) CROSS_COMPILE=$(toolchain)
```
---

## Building Example
```bash
sudo apt install aarch64-linux-gnu
git clone https://github.com/ivoszbg/uniLoader
cd uniLoader
cp /home/user/linux/arch/arm64/boot/Image blob/Image
cp /home/user/linux/arch/arm64/boot/dts/exynos/exynos8895-dreamlte.dtb blob/dtb
cp /home/user/ramdisk.gz blob/ramdisk
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- dreamlte_defconfig
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu-
```

### Build with Clang and LLVM toolchain
```bash
make ARCH=aarch64 LLVM=1 dreamlte_defconfig
make ARCH=aarch64 LLVM=1
```

```bash
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- LLVM=/bin dreamlte_defconfig
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- LLVM=/bin
```

---

## License
This project is licensed under GPLv2.
