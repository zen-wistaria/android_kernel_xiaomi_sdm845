# Panduan Lengkap Kompilasi Kernel & AnyKernel3 (ResukiSU Poco F1)

Dokumen ini menjelaskan langkah-langkah lengkap dari awal hingga akhir untuk mempersiapkan toolchain, melakukan kompilasi binary pendukung, melakukan kompilasi kernel, dan mengemas hasil build menjadi file `.zip` flashable via Recovery.

---

## 1. Persiapan Awal: Mengunduh Toolchain Proton Clang
Kernel ini dikompilasi menggunakan toolchain **Proton Clang**. Unduh/clone repository Proton Clang terlebih dahulu ke direktori `~/Coding/`:

```bash
git clone --depth=1 https://github.com/kdrag0n/proton-clang.git ~/Coding/proton-clang
```

---

## 2. Menyesuaikan Path Toolchain
Jika Anda menaruh folder `proton-clang` di tempat lain, buka file `build.sh` yang ada di root project kernel ini dan sesuaikan variabel `TC` pada baris ke-7:

```bash
# Contoh jika diletakkan di ~/Coding/proton-clang
TC="$HOME/Coding/proton-clang"
```

---

## 3. Kompilasi Binary Pendukung (`ksu_sf`)
Sebelum mengemas kernel menggunakan AnyKernel3, Anda **wajib** mengompilasi binary `ksu_sf` terlebih dahulu karena program kemasan (`build-anykernel.sh`) akan memverifikasi keberadaannya.

Panduan langkah-langkah kompilasi mandiri `ksu_sf` secara detail dapat dilihat di:
* [susfs4ksu/BUILD_GUIDE_FIXED.md](file:///home/zen/Coding/android_kernel_xiaomi_sdm845/susfs4ksu/BUILD_GUIDE_FIXED.md)

Pastikan setelah proses kompilasi silang selesai, file binary hasil build diletakkan di:
* `susfs4ksu/ksu_susfs/jni/ksu_sf`

---

## 4. Kompilasi Kernel
Setelah prasyarat di atas siap, jalankan script kompilasi kernel:

```bash
./build.sh
```

Proses ini akan menghasilkan file kernel image di:
* `out/arch/arm64/boot/Image.gz-dtb`

---

## 5. Pengemasan AnyKernel3 (Membuat Flashable Zip)
Untuk mengemas kernel beserta binary `ksu_sf` menjadi paket `.zip` siap flash, jalankan script berikut:

```bash
./build-anykernel.sh
```

### Apa yang dilakukan script `build-anykernel.sh`?
1. **Verifikasi Keberadaan `ksu_sf`:** Mengecek keberadaan file binary `susfs4ksu/ksu_susfs/jni/ksu_sf`. Jika tidak ditemukan, proses build akan dibatalkan dengan peringatan agar Anda melakukan kompilasi binary terlebih dahulu.
2. **Sinkronisasi File:** 
   * Menyalin binary `ksu_sf` ke folder `AnyKernel3/ksu_sf`.
   * Menyalin kernel image `Image.gz-dtb` terbaru ke folder `AnyKernel3/Image.gz-dtb`.
3. **Pembuatan Paket Zip:** Mengompres folder AnyKernel3 menjadi flashable zip dan menyimpannya di direktori `~/Coding/` dengan format nama `ResukiSU-PocoF1-YYYYMMDD-HHMM.zip`.
