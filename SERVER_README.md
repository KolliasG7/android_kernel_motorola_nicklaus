# Moto E4 Plus (nicklaus) — Linux Server Kernel

## What changed vs stock

### defconfig (`nicklaus_server_defconfig`)
| What | Why |
|---|---|
| `CONFIG_MODULES=y` | Loadable kernel modules (was missing!) |
| `CONFIG_DEVTMPFS=y` | udev / systemd need this |
| `CONFIG_NAMESPACES` + NS variants | systemd, containers |
| `CONFIG_OVERLAY_FS=y` | rootfs for most distros |
| `CONFIG_SQUASHFS=y` | initramfs / postmarketOS rootfs |
| `CONFIG_USB_CONFIGFS` + RNDIS/ECM | SSH over USB cable |
| `CONFIG_MODULES` + crypto suite | OpenSSH works fully |
| `CONFIG_ZRAM=y` + `CONFIG_SWAP=y` | Swap on compressed RAM |
| `CONFIG_CPU_FREQ_GOV_SCHEDUTIL=y` | Better governor for server |
| `CONFIG_BPF_SYSCALL=y` | Modern tools (bpftrace, etc.) |
| `CONFIG_IO_URING=y` | Fast async I/O |
| Android binder/LMK disabled | No Android junk |
| vmalloc fixed: 496M → 512M | Correct for 3GB device |
| Root changed to `/dev/mmcblk0p22` | Boot from eMMC, not ramdisk |

### DTS (`nicklaus_server.dts`)
| What | Why |
|---|---|
| RAM: `0x1F000000` → `0xC0000000` | Was 496MB! Now correct 3GB |
| CPU clocks: 1.1GHz → 1.45GHz | MT6737T correct max freq |

---

## How to build

```bash
# Install cross compiler (Debian/Ubuntu)
sudo apt install gcc-aarch64-linux-gnu

# Build
chmod +x build_server_kernel.sh
./build_server_kernel.sh
```

Output: `out_server/arch/arm64/boot/Image.gz-dtb`

---

## How to deploy

### Step 1: Get a minimal ARM64 rootfs
```bash
# Alpine Linux (smallest, good for server)
wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/aarch64/alpine-minirootfs-*-aarch64.tar.gz

# OR Debian minimal ARM64
debootstrap --arch=arm64 bookworm /mnt/rootfs http://deb.debian.org/debian
```

### Step 2: Set up SSH in rootfs
```bash
# Inside rootfs (chroot or after boot):
apk add openssh  # Alpine
# OR
apt install openssh-server  # Debian

# Enable and start
rc-update add sshd  # Alpine
# OR
systemctl enable ssh  # Debian
```

### Step 3: Build boot image
```bash
# Get original boot.img from your device (via adb)
adb pull /dev/block/bootdevice/by-name/boot boot_orig.img

# Unpack to get ramdisk
python3 unpackbootimg.py -i boot_orig.img

# Repack with new kernel + your initramfs
mkbootimg \
  --kernel out_server/arch/arm64/boot/Image.gz-dtb \
  --ramdisk initramfs.cpio.gz \
  --cmdline "console=ttyMT3,921600n1 root=/dev/mmcblk0p22 rw rootwait init=/sbin/init" \
  --base 0x40000000 \
  --output server_boot.img
```

### Step 4: Flash
```bash
# Boot into fastboot (Power + Vol Down)
fastboot flash boot server_boot.img
fastboot reboot
```

### Step 5: Connect via SSH over USB
```bash
# Phone creates USB network interface (RNDIS)
# Phone IP usually: 192.168.42.129
ssh root@192.168.42.129
```

---

## WiFi SSH (alternative)
Once booted, configure wpa_supplicant for WiFi:
```bash
wpa_passphrase "YourWiFi" "password" > /etc/wpa_supplicant.conf
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
udhcpc -i wlan0  # Alpine
# Then SSH via WiFi IP
```

---

## Notes
- eMMC partition `/dev/mmcblk0p22` = userdata on most MT6737 Moto devices. Verify with `ls -la /dev/block/platform/*/by-name/` before flashing.
- WiFi driver is MTK combo chip — may need firmware blob from stock ROM.
- No display needed — headless server only.
