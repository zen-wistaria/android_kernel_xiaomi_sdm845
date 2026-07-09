# Panduan Kompilasi Mandiri `ksu_susfs` (Fixed Version) untuk ReSuKisu

Panduan ini mendokumentasikan cara mengompilasi alat baris perintah (`CLI tool`) `ksu_susfs` yang telah disesuaikan agar berjalan sempurna pada kernel dengan integrasi **ReSuKisu** (inline hook manual) + **SUSFS v1.5.5** tanpa memicu pesan error *"SUSFS command execution failed"* atau *"file not found"* pada manager.

---

## 1. Latar Belakang Masalah
Pada integrasi ReSuKisu, perintah SUSFS dikirim via hook syscall `reboot` (karena keterbatasan inline hook manual). Syscall `reboot` hanya menerima **4 argumen**, sedangkan aplikasi manager secara bawaan memanggil syscall `prctl` dengan **5 argumen** (argumen ke-5 berisi alamat memori variabel `error` di userspace).

Karena argumen ke-5 tidak pernah sampai ke kernel, variabel `error` di userspace selalu bernilai `-1` (default). Ini menyebabkan CLI tool bawaan selalu mengembalikan exit code error (`126`), sehingga UI manager menampilkan pesan gagal dan menolak menyimpan konfigurasi.

Selain itu, struktur waktu (`struct st_susfs_sus_kstat`) pada kernel Xiaomi 4.9 sering kali memiliki urutan kolom waktu yang berbeda dengan binary prebuilt bawaan manager, menyebabkan data waktu file menjadi bergeser/korup.

---

## 2. Solusi Pemecahan Masalah
1. **Redireksi Makro `prctl` di Userspace:**
   Kita mengganti makro `prctl` di dalam kode sumber `main.c` agar memetakan panggilan ke syscall `reboot` secara manual, lalu menuliskan langsung nilai kembaliannya ke variabel `error` di userspace.
2. **Pengalihan Eksekusi di Sisi Kernel:**
   Karena manager akan selalu menulis ulang file `/data/adb/ksu/bin/ksu_susfs` saat startup (dan kita tidak boleh me-write-protect file tersebut agar tidak memicu error *"file not found"*), kita melakukan **redireksi eksekusi secara in-place** di dalam kernel (file `sucompat.c`).
3. **Nama File Lebih Pendek (`ksu_sf`):**
   Kernel mengalihkan `/data/adb/ksu/bin/ksu_susfs` (27 karakter) ke `/data/adb/ksu/bin/ksu_sf` (24 karakter) secara in-place menggunakan `strcpy`. Karena string baru lebih pendek, proses ini aman dari crash memori (buffer overflow) dan bebas dari Kernel Panic.

---

## 3. Prasyarat Kompilasi di PC Linux (Kali/Ubuntu/Debian)
Pasang cross-compiler ARM64 dan header libc cross-compile terlebih dahulu:
```bash
sudo apt update
sudo apt install -y gcc-aarch64-linux-gnu libc6-dev-arm64-cross
```

---

## 4. Modifikasi pada Kode Sumber `main.c`
File yang dimodifikasi berada di `ksu_susfs/jni/main.c`. Berikut adalah perubahan penting yang diterapkan:

1. **Nonaktifkan Header Android Log:**
   Karena kita melakukan kompilasi silang menggunakan GCC standar (glibc), bukan NDK (bionic), matikan header `<android/log.h>` yang tidak kompatibel:
   ```c
   // #include <android/log.h>
   ```

2. **Tambahkan Header `<limits.h>` & Kompatibilitas `timespec` glibc:**
   Tambahkan kode berikut di bawah baris include untuk mendukung makro waktu stat pada glibc:
   ```c
   #include <limits.h>

   #ifndef st_atime_nsec
   #define st_atime_nsec st_atim.tv_nsec
   #endif
   #ifndef st_mtime_nsec
   #define st_mtime_nsec st_mtim.tv_nsec
   #endif
   #ifndef st_ctime_nsec
   #define st_ctime_nsec st_ctim.tv_nsec
   #endif
   #ifndef st_atimensec
   #define st_atimensec st_atim.tv_nsec
   #endif
   #ifndef st_mtimensec
   #define st_mtimensec st_mtim.tv_nsec
   #endif
   #ifndef st_ctimensec
   #define st_ctimensec st_ctim.tv_nsec
   #endif
   ```

3. **Mendefinisikan Ulang Makro `prctl`:**
   Tambahkan logika redireksi reboot berikut agar variabel `error` userspace terupdate secara lokal:
   ```c
   #ifdef prctl
   #undef prctl
   #endif
   #define prctl(option, arg2, arg3, arg4, arg5) ({ \
       int _r = syscall(__NR_reboot, option, 0xFAFAFAFA, arg2, arg3); \
       if (arg5) *(int*)(arg5) = _r; \
       _r; \
   })
   ```

---

## 5. Langkah-Langkah Kompilasi Mandiri
Masuk ke direktori `ksu_susfs/jni` tempat file `main.c` berada, lalu jalankan kompilasi statis menggunakan `aarch64-linux-gnu-gcc`:

```bash
cd ksu_susfs/jni
aarch64-linux-gnu-gcc -static -O3 main.c -o ksu_sf
```

*Catatan:* Hasil kompilasi berupa binary statis bernama `ksu_sf`.

---

## 6. Pemasangan pada Perangkat (Manual)
Kirimkan binary `ksu_sf` hasil kompilasi ke direktori bin KernelSU di perangkat Anda:

```bash
adb push ksu_sf /data/adb/ksu/bin/ksu_sf
adb shell chmod 755 /data/adb/ksu/bin/ksu_sf
adb shell chown root:root /data/adb/ksu/bin/ksu_sf
```

---

## 7. Integrasi di Sisi Kernel (Wajib)
Agar kernel mengalihkan pemanggilan binary otomatis ke file `ksu_sf` yang kita buat, pastikan Anda menambahkan kode berikut pada file `KernelSU/kernel/feature/sucompat.c` di dalam fungsi `ksu_handle_execveat` (sebelum memanggil `ksu_handle_execve`):

```c
#ifdef CONFIG_KSU_SUSFS
    if (filename && filename->name && !strcmp(filename->name, "/data/adb/ksu/bin/ksu_susfs")) {
        strcpy((char *)filename->name, "/data/adb/ksu/bin/ksu_sf");
    }
#endif
```
