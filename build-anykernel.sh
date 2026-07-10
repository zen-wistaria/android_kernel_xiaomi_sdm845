#!/bin/bash
set -e

export ANYKERNEL=$(pwd)/AnyKernel3
export PLACE=$HOME/Coding
NAME=ResukiSU-PocoF1-$(date +%Y%m%d-%H%M)
# KSU_SF_BIN=$(pwd)/susfs4ksu/ksu_susfs/jni/ksu_sf
KSU_SF_BIN=$(pwd)/KernelSU/kernel/ksu_sf
if [ ! -f "$KSU_SF_BIN" ]; then
  echo "=========================================================="
  echo "ERROR: Binary 'ksu_sf' tidak ditemukan!"
  echo "Silakan lakukan kompilasi terlebih dahulu pada binary"
  echo "ksu_susfs yang sudah difix dengan merujuk ke panduan di:"
  echo "  susfs4ksu/BUILD_GUIDE_FIXED.md"
  echo "=========================================================="
  exit 1
fi

IMAGE=out/arch/arm64/boot/Image.gz-dtb
cp -r $IMAGE $ANYKERNEL/Image.gz-dtb
cd $ANYKERNEL
zip -r9 $PLACE/$NAME.zip . -x "*.git*" -x "LICENCE" -x "README.md"
