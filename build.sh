#!/bin/bash
set -e

export ARCH=arm64
export SUBARCH=arm64
export KSU=1

TC="$HOME/Coding/proton-clang"

# ==== Touch changed files so build system detects them ====
touch KernelSU/kernel/runtime/boot_event.c

# ==== Rebuild ksu_sf userspace binary ====
echo ">>> Building ksu_sf binary..."
aarch64-linux-gnu-gcc -static -O2 -s -std=gnu11 \
  -I susfs4ksu/ksu_susfs/jni \
  -o KernelSU/kernel/ksu_sf \
  susfs4ksu/ksu_susfs/jni/main.c 2>&1
chmod 755 KernelSU/kernel/ksu_sf 2>/dev/null

# ==== Merge defconfig ====
mkdir -p out
scripts/kconfig/merge_config.sh -O out \
  arch/arm64/configs/vendor/xiaomi/mi845_defconfig \
  arch/arm64/configs/vendor/xiaomi/beryllium.config || true

make ARCH=arm64 O=out oldconfig

# ==== Compile ====
export LD_LIBRARY_PATH="$TC/lib:$LD_LIBRARY_PATH"

make -j$(nproc) O=out \
  ARCH=arm64 \
  CC="$TC/bin/clang" \
  CLANG_TRIPLE=aarch64-linux-gnu- \
  CROSS_COMPILE="$TC/bin/aarch64-linux-gnu-" \
  CROSS_COMPILE_ARM32=arm-linux-gnueabi- \
  AR="$TC/bin/llvm-ar" \
  NM="$TC/bin/llvm-nm" \
  OBJCOPY="$TC/bin/llvm-objcopy" \
  OBJDUMP="$TC/bin/llvm-objdump" \
  STRIP="$TC/bin/llvm-strip" \
  LLVM_IAS=1
