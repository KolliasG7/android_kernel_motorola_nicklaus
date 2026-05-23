#!/bin/bash
# Build script - Moto E4 Plus (nicklaus) - Linux Server Kernel
# Architecture: ARM (32-bit) - required because the MT6735 bootloader (Little Kernel)
# is 32-bit and cannot hand off to an ARM64 kernel image.
#
# Uses: arch/arm/configs/nicklaus_server_defconfig + arch/arm/boot/dts/nicklaus_server.dts

set -e

KERNEL_DIR=$(pwd)
OUT_DIR="${KERNEL_DIR}/out_server"
CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"
ARCH=arm
JOBS="${JOBS:-$(nproc)}"

echo "[*] Building nicklaus server kernel (ARM 32-bit)..."
echo "[*] CROSS_COMPILE=${CROSS_COMPILE}"
echo "[*] Jobs: ${JOBS}"

mkdir -p "${OUT_DIR}"

# Always wipe stale dtc build artifacts — old .o files cause yylloc duplicate
# definition error with GCC 10+ even after patching the source
rm -rf "${OUT_DIR}/scripts/dtc"

# Configure
make O="${OUT_DIR}" \
     ARCH="${ARCH}" \
     CROSS_COMPILE="${CROSS_COMPILE}" \
     nicklaus_server_defconfig || { echo "[!] Config FAILED"; exit 1; }

# Build kernel + appended DTB
make O="${OUT_DIR}" \
     ARCH="${ARCH}" \
     CROSS_COMPILE="${CROSS_COMPILE}" \
     -j"${JOBS}" \
     zImage-dtb 2>&1 | tee "${OUT_DIR}/build.log"

BUILD_EXIT=${PIPESTATUS[0]}
if [ "${BUILD_EXIT}" -ne 0 ]; then
    echo ""
    echo "[!] BUILD FAILED (exit ${BUILD_EXIT}) — check ${OUT_DIR}/build.log"
    exit "${BUILD_EXIT}"
fi

KERNEL_IMG="${OUT_DIR}/arch/arm/boot/zImage-dtb"
if [ ! -f "${KERNEL_IMG}" ]; then
    # Some MTK trees produce zImage only; DTB must be appended manually
    ZIMAGE="${OUT_DIR}/arch/arm/boot/zImage"
    DTB="${OUT_DIR}/arch/arm/boot/dts/nicklaus_server.dtb"
    if [ ! -f "${DTB}" ]; then
        echo "[!] BUILD FAILED — neither zImage-dtb nor zImage+DTB found."
        exit 1
    fi
    echo "[*] Appending DTB manually..."
    cat "${ZIMAGE}" "${DTB}" > "${OUT_DIR}/arch/arm/boot/zImage-dtb"
    KERNEL_IMG="${OUT_DIR}/arch/arm/boot/zImage-dtb"
fi

echo ""
echo "[+] Build SUCCEEDED"
echo "[+] Kernel image: ${KERNEL_IMG}"
echo ""
echo "Next: wrap with mkbootimg and flash via fastboot."
echo "See SERVER_README.md for full instructions."
