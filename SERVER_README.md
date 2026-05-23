# Moto E4 Plus (nicklaus) - Bare Linux Server Kernel

> **Architecture note:** The MT6735/MT6737 SoC is ARMv8-capable hardware,
> but Motorola shipped it with a **32-bit Little Kernel (LK)** bootloader.
> A 32-bit LK cannot hand off to an ARM64 kernel. The server build therefore
> targets **ARCH=arm (ARMv7/Thumb-2, 32-bit)** - you still get a full Linux
> server environment, just without the 64-bit register width.

---

## Prerequisites

```bash
# Arch Linux
sudo pacman -S arm-linux-gnueabihf-gcc make bc dtc android-tools

# Ubuntu/Debian
sudo apt install gcc-arm-linux-gnueabihf make bc device-tree-compiler android-tools-fastboot
```

---

## Build

```bash
git clone https://github.com/KolliasG7/android_kernel_motorola_nicklaus -b nougat
cd android_kernel_motorola_nicklaus

# Optional: override compiler
export CROSS_COMPILE=arm-linux-gnueabihf-
export JOBS=$(nproc)

bash build_server_kernel.sh
```

Output: `out_server/arch/arm/boot/zImage-dtb`

---

## Create boot image

```bash
mkbootimg \
  --kernel out_server/arch/arm/boot/zImage-dtb \
  --ramdisk initramfs.cpio.gz \
  --cmdline "console=ttyMT3,921600n1 console=tty0 root=/dev/ram rw init=/sbin/init earlyprintk ignore_loglevel" \
  --base 0x40000000 \
  --pagesize 2048 \
  --output server_boot.img
```

### Alpine Linux ramdisk (recommended, ~8MB)

```bash
# Download Alpine armv7 minirootfs
wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/armv7/alpine-minirootfs-*-armv7.tar.gz

mkdir rootfs && cd rootfs
tar xf ../alpine-minirootfs-*-armv7.tar.gz

# Pack
find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz
cd ..
```

---

## Flash

```bash
# Temporary boot (does not overwrite stock):
fastboot boot server_boot.img

# Permanent:
fastboot flash boot server_boot.img
```

Restore stock: `fastboot flash boot stock_boot.img`

---

## Connect via USB RNDIS (SSH over USB)

1. Connect USB cable to PC
2. Phone presents as `usb0` / `eth0` at `192.168.42.129`

```bash
# On your PC:
sudo ip addr add 192.168.42.100/24 dev usb0
sudo ip link set usb0 up
ssh root@192.168.42.129
```

---

## Known issues / tips

- **Only 496MB RAM?** Make sure you used `arch/arm/boot/dts/nicklaus_server.dts`
  which fixes the memory node to report the full 3GB.
- **Fastboot stuck after boot attempt?** Verify `file zImage-dtb` shows `ARM`
  not `ARM64`. An ARM64 image will always fail on this device's 32-bit LK.
- **init not found?** Alpine uses `/sbin/init`. Adjust `init=` in cmdline if
  using a different distro.
- **WiFi** requires MTK firmware blobs from stock ROM: `adb pull /system/etc/firmware/ rootfs/lib/firmware/`
