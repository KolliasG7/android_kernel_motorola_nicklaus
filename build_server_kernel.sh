#!/bin/bash
# Build script - Moto E4 Plus (nicklaus) - Linux Server Kernel
# Uses: nicklaus_server_defconfig + nicklaus_server.dts

set -e

KERNEL_DIR=$(pwd)
OUT_DIR="${KERNEL_DIR}/out_server"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
ARCH=arm64
JOBS=$(nproc)

echo "[*] Building nicklaus server kernel..."
echo "[*] CROSS_COMPILE=${CROSS_COMPILE}"
echo "[*] Jobs: ${JOBS}"

mkdir -p "${OUT_DIR}"

# Configure
make O="${OUT_DIR}" \
     ARCH="${ARCH}" \
     CROSS_COMPILE="${CROSS_COMPILE}" \
     nicklaus_server_defconfig

# Build
make O="${OUT_DIR}" \
     ARCH="${ARCH}" \
     CROSS_COMPILE="${CROSS_COMPILE}" \
     -j"${JOBS}" \
     Image.gz-dtb 2>&1 | tee "${OUT_DIR}/build.log"

echo ""
echo "[+] Build done!"
echo "[+] Kernel image: ${OUT_DIR}/arch/arm64/boot/Image.gz-dtb"
echo ""
echo "[!] Flash via fastboot:"
echo "    fastboot flash boot <your_boot.img>"
echo "    (use mkbootimg to wrap Image.gz-dtb with initramfs)"
