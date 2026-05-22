# Moto E4 Plus (nicklaus) — Linux Server Guide
> Arch Linux host | ARM64 target | SSH-only headless server

---

## Table of Contents
1. [Prerequisites](#1-prerequisites)
2. [Clone the repo](#2-clone-the-repo)
3. [Build the kernel](#3-build-the-kernel)
4. [Prepare the rootfs](#4-prepare-the-rootfs)
5. [Find your eMMC partitions](#5-find-your-emmc-partitions)
6. [Build the boot image](#6-build-the-boot-image)
7. [Flash and boot](#7-flash-and-boot)
8. [First boot setup](#8-first-boot-setup)
9. [Connect via SSH](#9-connect-via-ssh)
10. [WiFi SSH (optional)](#10-wifi-ssh-optional)
11. [Tips and notes](#11-tips-and-notes)

---

## 1. Prerequisites

Install everything you need on Arch:

```bash
sudo pacman -S base-devel bc dtc aarch64-linux-gnu-gcc android-tools git
```

> `android-tools` gives you `adb`, `fastboot`, and `mkbootimg`.

Verify cross compiler works:
```bash
aarch64-linux-gnu-gcc --version
# should print something like: aarch64-linux-gnu-gcc (GCC) 13.x.x
```

---

## 2. Clone the repo

```bash
git clone https://github.com/KolliasG7/android_kernel_motorola_nicklaus.git
cd android_kernel_motorola_nicklaus
```

The server-specific files are already on the `nougat` branch (default):
- `arch/arm64/configs/nicklaus_server_defconfig` — kernel config
- `arch/arm64/boot/dts/nicklaus_server.dts` — device tree (RAM + clocks fixed)
- `build_server_kernel.sh` — build script

---

## 3. Build the kernel

```bash
chmod +x build_server_kernel.sh
./build_server_kernel.sh
```

This will:
- Configure the kernel with `nicklaus_server_defconfig`
- Cross-compile for ARM64
- Output kernel image to `out_server/arch/arm64/boot/Image.gz-dtb`

Build takes ~10-20 minutes depending on your CPU.

If you want to use all your cores explicitly:
```bash
JOBS=16 ./build_server_kernel.sh   # replace 16 with your core count
```

---

## 4. Prepare the rootfs

Pick one. Alpine is smallest and best for a server.

### Option A — Alpine Linux (recommended, ~8MB)

```bash
# Download Alpine minirootfs for aarch64
wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/aarch64/alpine-minirootfs-3.20.0-aarch64.tar.gz

# Create rootfs dir and extract
mkdir rootfs
sudo tar -xzf alpine-minirootfs-3.20.0-aarch64.tar.gz -C rootfs
```

### Option B — Debian minimal (~300MB)

```bash
sudo pacman -S debootstrap  # or from AUR: yay -S debootstrap
sudo debootstrap --arch=arm64 bookworm rootfs http://deb.debian.org/debian
```

---

### Set up SSH inside the rootfs

We use QEMU to chroot into the ARM64 rootfs from your x86 Arch machine:

```bash
# Install QEMU user static
sudo pacman -S qemu-user-static

# Copy the QEMU binary into rootfs so it can run ARM64 binaries
sudo cp /usr/bin/qemu-aarch64-static rootfs/usr/bin/

# Mount required pseudo filesystems
sudo mount --bind /dev rootfs/dev
sudo mount --bind /proc rootfs/proc
sudo mount --bind /sys rootfs/sys

# Chroot into rootfs
sudo chroot rootfs /bin/sh
```

Inside the chroot, run:

**Alpine:**
```sh
# Set up basic system
echo "nameserver 8.8.8.8" > /etc/resolv.conf
apk update
apk add openssh openrc util-linux

# Set root password
passwd root
# Enter a password you'll remember

# Enable SSH on boot
rc-update add sshd
rc-update add networking

# Configure SSH to allow root login
echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
echo "PasswordAuthentication yes" >> /etc/ssh/sshd_config

# Basic network config (USB RNDIS — phone side)
cat > /etc/network/interfaces << 'EOF'
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
    address 192.168.42.129
    netmask 255.255.255.0
    gateway 192.168.42.1
EOF

exit
```

**Debian:**
```sh
apt update
apt install -y openssh-server systemd-sysv

passwd root

echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
systemctl enable ssh

exit
```

Unmount after chroot:
```bash
sudo umount rootfs/dev rootfs/proc rootfs/sys
```

---

### Pack the rootfs into initramfs

```bash
cd rootfs
sudo find . | sudo cpio -H newc -o | gzip > ../initramfs.cpio.gz
cd ..
ls -lh initramfs.cpio.gz   # should be a few MB
```

---

## 5. Find your eMMC partitions

Before flashing anything, check partition layout on the phone.

**Boot phone normally (stock Android), enable USB debugging, then:**

```bash
adb shell
# Inside phone shell:
ls -la /dev/block/platform/*/by-name/
```

Look for partitions like:
- `boot` — where kernel lives
- `userdata` — where you'll put the rootfs (largest partition, usually)
- `system` — Android system (you can wipe this for more space)

Note down the `/dev/block/mmcblkXpY` paths for `boot` and `userdata`.

> **Common layout on MT6737 Moto devices:**
> - boot = `/dev/mmcblk0p7` (or similar)
> - userdata = `/dev/mmcblk0p22` (or similar)

---

## 6. Build the boot image

You need the original `boot.img` to extract the correct base address and ramdisk offset:

```bash
# Pull original boot image from phone
adb pull /dev/block/platform/$(adb shell ls /dev/block/platform/)/by-name/boot boot_orig.img
```

Unpack it to get header info:
```bash
# Install unpackbootimg (AUR)
yay -S android-bootimage-tools
# OR use Python tool:
pip install --user android-bootimg

unpackbootimg -i boot_orig.img -o boot_orig_unpacked/
cat boot_orig_unpacked/boot_orig.img-cmdline  # see original cmdline
cat boot_orig_unpacked/boot_orig.img-base     # note this value
```

Repack with new kernel and your initramfs:
```bash
mkbootimg \
  --kernel out_server/arch/arm64/boot/Image.gz-dtb \
  --ramdisk initramfs.cpio.gz \
  --cmdline "console=ttyMT3,921600n1 console=tty0 root=/dev/ram rw init=/sbin/init" \
  --base 0x40000000 \
  --pagesize 2048 \
  --output server_boot.img

ls -lh server_boot.img
```

> If `--base` gives errors, use the value from `boot_orig.img-base` file.

---

## 7. Flash and boot

**Boot into fastboot mode:**
- Power off the phone
- Hold **Power + Volume Down** until fastboot screen appears

```bash
# Verify phone is detected
fastboot devices

# Flash the new boot image
fastboot flash boot server_boot.img

# Reboot
fastboot reboot
```

> **Safe option:** Use `fastboot boot server_boot.img` first (boots once without flashing) to test before committing.

---

## 8. First boot setup

Phone will boot to Linux. No display output visible — it's headless. Wait ~30 seconds for boot to complete.

Connect phone via USB cable to your PC.

The phone creates a USB network interface (RNDIS) on your PC:

```bash
# Check for new network interface
ip link show
# Should see something like: usb0, enp0s20f0u1, etc.

# Bring it up and assign PC-side IP
sudo ip link set usb0 up
sudo ip addr add 192.168.42.1/24 dev usb0
```

---

## 9. Connect via SSH

```bash
ssh root@192.168.42.129
# Enter the password you set during rootfs setup
```

You're in. 🗿

---

## 10. WiFi SSH (optional)

If you want to SSH over WiFi instead of USB:

Inside the phone (via USB SSH first):

```bash
# Alpine
apk add wpa_supplicant

# Create WiFi config
wpa_passphrase "YourWiFiSSID" "YourPassword" > /etc/wpa_supplicant/wpa_supplicant.conf

# Connect
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
udhcpc -i wlan0

# Check IP
ip addr show wlan0
```

Then from your PC:
```bash
ssh root@<phone-wifi-ip>
```

To make WiFi persistent on boot (Alpine):
```bash
rc-update add wpa_supplicant
# Add wlan0 to /etc/network/interfaces with dhcp
```

---

## 11. Tips and notes

### Performance
- 3GB RAM is plenty for a headless server
- `schedutil` governor (enabled in our config) scales CPU only when needed — saves battery
- Enable zram swap for extra headroom:
  ```bash
  modprobe zram
  echo lz4 > /sys/block/zram0/comp_algorithm
  echo 512M > /sys/block/zram0/disksize
  mkswap /dev/zram0
  swapon /dev/zram0
  ```

### Useful server software on Alpine
```bash
apk add nginx          # web server
apk add python3        # scripting
apk add git            # git server
apk add tmux           # terminal multiplexer
apk add htop           # resource monitor
apk add curl wget      # downloads
```

### Keep phone alive (no sleep)
```bash
# Prevent CPU sleep while running as server
echo 1 > /sys/power/wake_lock
```

### Charging while serving
Phone charges normally over USB even when running Linux — RNDIS and charging coexist fine.

### WiFi driver
MTK combo WiFi chip needs firmware blobs from stock ROM. If WiFi doesn't work:
```bash
# On stock Android before flashing:
adb pull /system/etc/firmware /tmp/fw
# Copy relevant *.bin files to rootfs /lib/firmware/
```

### Going back to Android
```bash
# Flash stock boot.img anytime
fastboot flash boot boot_orig.img
fastboot reboot
```

---

## Summary of what was changed in this kernel vs stock

| File | Change |
|---|---|
| `nicklaus_server_defconfig` | +modules, +devtmpfs, +namespaces, +overlayfs, +squashfs, +USB RNDIS, +SSH crypto, +zram, schedutil governor, disabled Android binder/LMK |
| `nicklaus_server.dts` | RAM bug fixed (496MB→3GB), CPU clocks fixed (1.1→1.45GHz) |
| `build_server_kernel.sh` | Arch-compatible build script |
