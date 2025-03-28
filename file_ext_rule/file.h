// https://github.com/SoyBeanMilkx/IO_Redirect/blob/main/src/file.h
#ifndef _KPM_FILE_H
#define _KPM_FILE_H

#include <linux/llist.h>
#include <ktypes.h>

#define PATH_MAX 256
#define MAX_LINES 10
#define MAX_INPUT_SIZE 512

struct vfsmount;
struct dentry;
struct mnt_idmap;
struct inode;

struct renamedata {
    struct mnt_idmap *old_mnt_idmap;
    struct inode *old_dir;
    struct dentry *old_dentry;
    struct mnt_idmap *new_mnt_idmap;
    struct inode *new_dir;
    struct dentry *new_dentry;
    struct inode **delegated_inode;
    unsigned int flags;
};

struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
};

struct file {
    union {
        struct llist_node    fu_llist;
        struct rcu_head      fu_rcuhead;
    } f_u;
    struct path     f_path;
    struct inode    *f_inode;
};

struct open_flags {
    int open_flag;
    umode_t mode;
    int acc_mode;
    int intent;
    int lookup_flags;
};

#endif //_KPM_FILE_H
