# uniLoader

**uniLoader** is a minimalistic loader, capable of booting Linux kernels.
It can be used as an intermediate bootloader, providing a clean booting
environment in case of a forced and buggy bootloader.

---

## Supported Architectures
- ARMv7
- ARMv8

---

## Supported Devices

### Apple
- N61AP

### Amazon
- PW3

### Google
- Taimen

### Huawei
- Agassi2

### itel
- P682LPN

### Infinix
- X6710

### Nokia
- Essential

### Nothing
- Tetris

### QEMU
- Virt

### Samsung
- A3XELTE
- A10
- A12S
- A30
- A33X
- BEYOND1LTE
- C1S
- DREAMLTE
- G0S
- GTA4XL
- HEROLTE
- J4LTE
- J5LTE
- JACKPOTLTE
- LUCKY7
- NOBLELTE
- R0Q
- R8S
- STARLTE
- X1S
- ZEROFLTE

### Volla
- Algiz

### Xiaomi
- Begonia
- Blossom
- Camellia

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

---

## License
This project is licensed under GPLv2.
