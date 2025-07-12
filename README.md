# uniLoader
A secondary bootloader that is capable of loading the upstream Linux kernel for Android and iOS-based devices.</br></br>
The purpose behind it is to provide a small shim for avoiding vendor bootloader quirks.</br>
> (ex.: some newer Exynos phones leave decon framebuffer refreshing disabled right before jumping to kernel, which makes initial debugging efforts when bringing up the platform to upstream linux hard)</br>
The currently supported architectures are ARMV7 and AARCH64.</br>

## Preparation
1. Install a toolchain that matches your target architecture (ex. AArch64 cross-toolchain):
    ```sh
    sudo apt install aarch64-linux-gnu
    ```
2. Clone the repository:
    ```sh
    git clone https://github.com/ivoszbg/uniLoader
    ```
3. Navigate to the project directory:
    ```sh
    cd uniLoader
    ```
4. Place your kernel, device tree and ramdisk blobs under the `blob/` directory.

### Make Syntax
```sh
make ARCH=<arch> CROSS_COMPILE=<toolchain>
```

### Building Example
```sh
sudo apt install aarch64-linux-gnu
git clone https://github.com/ivoszbg/uniLoader
cd uniLoader
cp /home/user/linux/arch/arm64/boot/Image blob/Image
cp /home/user/linux/arch/arm64/boot/dts/exynos/exynos8895-dreamlte.dtb blob/dtb
cp /home/user/ramdisk.gz blob/ramdisk
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- -j4 dreamlte_defconfig
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- -j4
```

### Usage

```sh
For Apple devices: Load the generated uniLoader binary via PongoOS.
For Android devices: Replace the Linux kernel in your boot.img with the uniLoader binary.
```

### TODO LIST
1. [C] Allow loading instead of PongoOS for Apple devices;
2. [C] Fix/replace the libc implementation;
3. [M] Implement a serial library and make the debug lib less hacky;
4. [M] Implement more features:
   > New devices</br>
   > Boot menu with countdown timer and selectable items via GPIO volume keys</br>
   > Threaded execution

<h6>
  [C] = Critical for overall ease of use and functionality</br>
  [M] = Nice to have :)
</h6>

### License

This project is licensed under GPL2.
