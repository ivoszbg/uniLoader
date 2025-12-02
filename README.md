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

### Infinix
- X6710

### Nokia
- Essential

### Nothing
- Tetris

### QEMU
- Virt

### Samsung
- A105F  
- A127F  
- A3XELTE  
- C1S  
- HEROLTE  
- G0S  
- NOBLELTE  
- JACKPOTLTE  
- ZEROFLTE  
- DREAMLTE  
- STARLTE  
- X1S  
- J5LTE  
- J4LTE  
- GTA4XL  
- R0Q  
- R8S

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
