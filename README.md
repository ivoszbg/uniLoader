# uniLoader
A simple secondary bootloader that is capable of loading Linux for Android and iOS based devices.

# Building
1. Install aarch64-linux-gnu
2. ```git clone https://github.com/ivoszbg/uniLoader```
3. ```cd uniLoader```
4. Place your kernel and device tree blobs under blob/
5. ```make BOARD=(oem)-(board-codename) SOC=(device-soc)```<br />
# Building example
```sudo apt install aarch64-linux-gnu```<br />
```git clone https://github.com/ivoszbg/uniLoader```<br />
```cd uniLoader```<br />
```cp /home/user/linux/arch/arm64/boot/Image blob/Image```<br />
```cp /home/user/linux/arch/arm64/boot/dts/exynos/exynos8895-dreamlte.dtb blob/dtb```<br />
```make BOARD=samsung-dreamlte SOC=exynos8895```
# Usage
For Apple devices: Load the generated **uniLoader** binary via PongoOs (TODO: allow loading **instead** of PongoOs)<br /><br />
For Android devices: Replace the linux kernel in your *boot.img* with the **uniLoader** binary
# Credits
VDavid003 - for his sboot wrapper<br />
Quack723 - Contributor<br />
Ivoszbg - Making this huge mess of a bootloader
# License
This project is licensed under GPL2.
