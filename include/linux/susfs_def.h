#ifndef KSU_SUSFS_DEF_H
#define KSU_SUSFS_DEF_H

#include <linux/bits.h>
#include <linux/thread_info.h>

#define SUSFS_MAGIC 0xFAFAFAFA

/********/
/* ENUM */
/********/
/* shared with userspace ksu_susfs tool */
#define CMD_SUSFS_ADD_SUS_PATH 0x55550
#define CMD_SUSFS_ADD_SUS_MOUNT 0x55560
#define CMD_SUSFS_ADD_SUS_KSTAT 0x55570
#define CMD_SUSFS_UPDATE_SUS_KSTAT 0x55571
#define CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY 0x55572
#define CMD_SUSFS_ADD_TRY_UMOUNT 0x55580
#define CMD_SUSFS_SET_UNAME 0x55590
#define CMD_SUSFS_ENABLE_LOG 0x555a0
#define CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG 0x555b0
#define CMD_SUSFS_ADD_OPEN_REDIRECT 0x555c0
#define CMD_SUSFS_ADD_SUS_PATH_LOOP 0x555c1
#define CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS 0x555c2
#define CMD_SUSFS_ADD_SUS_MAP 0x555c3
#define CMD_SUSFS_ENABLE_AVC_LOG_SPOOFING 0x555c4
#define CMD_SUSFS_RUN_UMOUNT_FOR_CURRENT_MNT_NS 0x555d0
#define CMD_SUSFS_SHOW_VERSION 0x555e1
#define CMD_SUSFS_SHOW_ENABLED_FEATURES 0x555e2
#define CMD_SUSFS_SHOW_VARIANT 0x555e3
#define CMD_SUSFS_SHOW_SUS_SU_WORKING_MODE 0x555e4
#define CMD_SUSFS_IS_SUS_SU_READY 0x555f0
#define CMD_SUSFS_SUS_SU 0x60000

#define SUSFS_MAX_LEN_PATHNAME 256
#define SUSFS_FAKE_CMDLINE_OR_BOOTCONFIG_SIZE 4096

#define TRY_UMOUNT_DEFAULT 0
#define TRY_UMOUNT_DETACH 1

#define SUS_SU_DISABLED 0
#define SUS_SU_WITH_OVERLAY 1
#define SUS_SU_WITH_HOOKS 2

#define DEFAULT_SUS_MNT_ID 100000
#define DEFAULT_SUS_MNT_ID_FOR_KSU_PROC_UNSHARE 1000000
#define DEFAULT_SUS_MNT_GROUP_ID 1000

/****************/
/* KSTAT FLAGS  */
/****************/
/* Shared field: 'flags' (was 'is_statically') in struct st_susfs_sus_kstat */
#define KSTAT_FLAG_IS_STATICALLY  BIT(0)  /* informational: added via add_sus_kstat_statically */
#define KSTAT_SPOOF_DEV           BIT(1)
#define KSTAT_SPOOF_INO           BIT(2)
#define KSTAT_SPOOF_NLINK         BIT(3)
#define KSTAT_SPOOF_SIZE          BIT(4)
#define KSTAT_SPOOF_ATIME         BIT(5)
#define KSTAT_SPOOF_MTIME         BIT(6)
#define KSTAT_SPOOF_CTIME         BIT(7)
#define KSTAT_SPOOF_BLOCKS        BIT(8)
#define KSTAT_SPOOF_BLKSIZE       BIT(9)
/* If flags == 0 or only KSTAT_FLAG_IS_STATICALLY set → spoof ALL fields (legacy compat) */

/*
 * inode->i_state => storing flag 'INODE_STATE_'
 * mount->mnt.susfs_mnt_id_backup => storing original mnt_id of normal mounts or custom sus mnt_id of sus mounts
 * task_struct->susfs_last_fake_mnt_id => storing last valid fake mnt_id
 * task_struct->susfs_task_state => storing flag 'TASK_STRUCT_'
 */

#define INODE_STATE_SUS_PATH BIT(24)
#define INODE_STATE_SUS_MOUNT BIT(25)
#define INODE_STATE_SUS_KSTAT BIT(26)
#define INODE_STATE_OPEN_REDIRECT BIT(27)

#define TASK_STRUCT_NON_ROOT_USER_APP_PROC BIT(24)
#define TASK_STRUCT_UMOUNTED BIT(25)
#define TIF_PROC_UMOUNTED 33

#define AS_FLAGS_SUS_MAP 39

struct st_susfs_sus_map {
	char target_pathname[256];
	int err;
};

/* v1.5.5 compat: use susfs_task_state (task_struct field) instead of TIF_PROC_UMOUNTED (thread_info flag) */
static inline bool susfs_is_current_proc_umounted_app(void) {
	return (likely(current->susfs_task_state & TASK_STRUCT_NON_ROOT_USER_APP_PROC) &&
			current_uid().val >= 10000);
}

#define SUSFS_IS_INODE_SUS_MAP(inode) \
	inode && inode->i_mapping && \
	unlikely(test_bit(AS_FLAGS_SUS_MAP, &inode->i_mapping->flags)) && \
	susfs_is_current_proc_umounted_app()

#define MAGIC_MOUNT_WORKDIR "/debug_ramdisk/workdir"
#define DATA_ADB_UMOUNT_FOR_ZYGOTE_SYSTEM_PROCESS "/data/adb/susfs_umount_for_zygote_system_process"
#define DATA_ADB_NO_AUTO_ADD_SUS_BIND_MOUNT "/data/adb/susfs_no_auto_add_sus_bind_mount"
#define DATA_ADB_NO_AUTO_ADD_SUS_KSU_DEFAULT_MOUNT "/data/adb/susfs_no_auto_add_sus_ksu_default_mount"
#define DATA_ADB_NO_AUTO_ADD_TRY_UMOUNT_FOR_BIND_MOUNT "/data/adb/susfs_no_auto_add_try_umount_for_bind_mount"

#endif // #ifndef KSU_SUSFS_DEF_H
