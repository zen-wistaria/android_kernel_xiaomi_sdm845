#!/bin/bash
set -e

export ARCH=arm64
export SUBARCH=arm64

TC="$HOME/Coding/proton-clang"

# ==== Merge defconfig — PATH default sistem, aman ====
mkdir -p out
scripts/kconfig/merge_config.sh -O out \
  arch/arm64/configs/vendor/xiaomi/mi845_defconfig \
  arch/arm64/configs/vendor/xiaomi/beryllium.config || true

make ARCH=arm64 O=out oldconfig

# ==== Compile — semua tool proton-clang pakai absolute path ====
export LD_LIBRARY_PATH="$TC/lib:$LD_LIBRARY_PATH"

#make -j$(nproc) O=out \
#  ARCH=arm64 \
#  CC="$TC/bin/clang" \
#  CLANG_TRIPLE=aarch64-linux-gnu- \
#  CROSS_COMPILE="$TC/bin/aarch64-linux-gnu-" \
#  CROSS_COMPILE_ARM32="$TC/bin/arm-linux-gnueabi-" \
#  AR="$TC/bin/llvm-ar" \
#  NM="$TC/bin/llvm-nm" \
#  OBJCOPY="$TC/bin/llvm-objcopy" \
#  OBJDUMP="$TC/bin/llvm-objdump" \
#  STRIP="$TC/bin/llvm-strip" \
#  LLVM=1 \
#  LLVM_IAS=1

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
