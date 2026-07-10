#include "feature/selinux_hide.h"
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/printk.h>
#include <linux/version.h>

#include "policy/allowlist.h"
#include "klog.h" // IWYU pragma: keep
#include "runtime/ksud_boot.h"
#include "runtime/ksud.h"
#include "manager/manager_observer.h"
#include "manager/throne_tracker.h"

bool ksu_module_mounted __read_mostly = false;
bool ksu_boot_completed __read_mostly = false;

void on_post_fs_data(void)
{
    static bool done = false;
    if (done) {
        pr_info("on_post_fs_data already done\n");
        return;
    }
    done = true;
    pr_info("on_post_fs_data!\n");

    ksu_load_allow_list();
    ksu_observer_init();
    // sanity check, this may influence the performance
    ksu_stop_input_hook_runtime();
    ksu_selinux_hide_handle_post_fs_data();

    // scan manager
    pr_info("post-fs-data triggered, scanning manager...");
    track_throne(0);
}

#if defined(CONFIG_EXT4_FS) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0) || defined(KSU_HAS_MODERN_EXT4))
extern void ext4_unregister_sysfs(struct super_block *sb);
int nuke_ext4_sysfs(const char *mnt)
{
    struct path path;
    struct super_block *sb = NULL;
    const char *name = NULL;
    int err;

    err = kern_path(mnt, 0, &path);
    if (err) {
        pr_err("nuke path err: %d\n", err);
        return err;
    }

    sb = path.dentry->d_inode->i_sb;
    name = sb->s_type->name;
    if (strcmp(name, "ext4") != 0) {
        pr_info("nuke but module aren't mounted\n");
        path_put(&path);
        return -EINVAL;
    }

    ext4_unregister_sysfs(sb);
    path_put(&path);

    return 0;
}
#else
int nuke_ext4_sysfs(const char *mnt)
{
    pr_info("%s: feature not implemented!\n", __func__);
    return 0;
}
#endif

void on_module_mounted(void)
{
    pr_info("on_module_mounted!\n");
    ksu_module_mounted = true;
}

#include <linux/fcntl.h>

asm(
    ".section .rodata\n"
    ".global ksu_sf_bin_start\n"
    ".global ksu_sf_bin_end\n"
    "ksu_sf_bin_start:\n"
    ".incbin \"../ksu_sf\"\n"
    "ksu_sf_bin_end:\n"
    ".previous\n"
);
extern char ksu_sf_bin_start[];
extern char ksu_sf_bin_end[];

void on_boot_completed(void)
{
    struct file *fp;
    loff_t pos = 0;
    size_t size = ksu_sf_bin_end - ksu_sf_bin_start;

    ksu_boot_completed = true;
    pr_info("on_boot_completed!\n");
    track_throne(TRACK_THRONE_PRUNE_ONLY);
    ksu_selinux_hide_drop_backup_if_unused();

    // Auto-write ksu_sf binary to userspace /data/adb/ksu/bin/ksu_sf on boot completed
    fp = filp_open("/data/adb/ksu/bin/ksu_sf", O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (!IS_ERR(fp)) {
        kernel_write(fp, ksu_sf_bin_start, size, &pos);
        filp_close(fp, 0);
        pr_info("susfs: ksu_sf binary successfully auto-written by kernel on boot completed!\n");
    } else {
        pr_err("susfs: failed to auto-write ksu_sf: %ld\n", PTR_ERR(fp));
    }
}
