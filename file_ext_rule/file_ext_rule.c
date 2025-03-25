// https://github.com/SoyBeanMilkx/IO_Redirect/blob/main/src/mod_main.c
#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <syscall.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <taskext.h>
#include "file.h"
#include "data_parse.h"

KPM_NAME("io_redirect");
KPM_VERSION("5.2.0");
KPM_AUTHOR("yuuki");
KPM_DESCRIPTION("Redirect target file to customized path");

typedef struct file *(*do_filp_open_func_t)(int dfd, struct filename *pathname, const struct open_flags *op);
static do_filp_open_func_t original_do_filp_open = NULL;
static do_filp_open_func_t backup_do_filp_open = NULL;
static struct file *replace_do_filp_open(int dfd, struct filename *pathname, const struct open_flags *op);

static hook_err_t hook_err = HOOK_NOT_HOOK;

char *(*d_path)(const struct path *path, char *buf, int buflen) = NULL;
void (*fput)(struct file *file) = NULL;

void *(*kf_vmalloc)(unsigned long size) = NULL;
void (*kf_vfree)(const void *addr) = NULL;

static char source_paths[MAX_LINES][PATH_MAX];
static char redirect_paths[MAX_LINES][PATH_MAX];
static int line_count = 0;


static inline void set_priv_selinx_allow(struct task_struct* task, int val) {
    struct task_ext* ext = get_task_ext(task);
    if (likely(task_ext_valid(ext))) {
        ext->priv_selinux_allow = val;
        dsb(ish);
    }
}

static struct file *replace_do_filp_open(int dfd, struct filename *pathname, const struct open_flags *op) {
    struct file *filp = backup_do_filp_open(dfd, pathname, op);
    if (likely(!IS_ERR(filp))) {
        char buf[PATH_MAX];
        memset(&buf, 0, PATH_MAX);
        char *currPath = d_path(&filp->f_path, buf, PATH_MAX);

        for (int i = 0; i < line_count; i++){

            if (strncmp(currPath, source_paths[i], strlen(source_paths[i])) == 0) {
                pr_info("[yuuki] Interception path %s successful, redirect to %s\n", source_paths[i], redirect_paths[i]);
                fput(filp);

                struct filename *new_pathname = kf_vmalloc(sizeof(struct filename));
                if (!new_pathname) {
                    return ERR_PTR(-ENOMEM);
                }
                new_pathname->name = redirect_paths[i];

                set_priv_selinx_allow(current, true);
                struct file *redirect_filp = backup_do_filp_open(dfd, new_pathname, op);
                set_priv_selinx_allow(current, false);

                kf_vfree(new_pathname);
                return redirect_filp;
            }

        }
    }
    return filp;
}

static inline bool hook_do_filp_open() {
    if (original_do_filp_open) {
        hook_err = hook((void *)original_do_filp_open, (void *)replace_do_filp_open, (void **)&backup_do_filp_open);
        if (hook_err != HOOK_NO_ERR) {
            pr_info("[yuuki] hook do_filp_open, %llx, error: %d\n", original_do_filp_open, hook_err);
        } else {
            return true;
        }
    } else {
        hook_err = HOOK_BAD_ADDRESS;
        pr_err("[yuuki] no symbol: do_filp_open\n");
    }
    return false;
}

static inline bool installHook() {
    bool ret = false;

    if (hook_err != HOOK_NO_ERR) {
        if (hook_do_filp_open()) {
            pr_info("[yuuki] hook installed...\n");
            ret = true;
        } else {
            pr_err("[yuuki] hook installation failed...\n");
        }
    } else {
        pr_info("[yuuki] hook already installed, skipping...\n");
        ret = true;
    }

    return ret;
}

static inline bool uninstallHook() {
    if (hook_err == HOOK_NO_ERR) {
        unhook((void *)original_do_filp_open);
        hook_err = HOOK_NOT_HOOK;
        pr_info("[yuuki] hook uninstalled...\n");
    } else {
        pr_info("[yuuki] Maybe it's not hooked, skipping...\n");
    }
    return true;
}

static inline bool control_internal(bool enable) {
    return enable ? installHook() : uninstallHook();
}

static long mod_init(const char *args, const char *event, void *__user reserved){
    pr_info("[yuuki] Initializing...\n");

    original_do_filp_open = (do_filp_open_func_t)kallsyms_lookup_name("do_filp_open");

    long ret = 0;

    pr_info("[yuuki] Kernel Version: %x\n", kver);
    pr_info("[yuuki] Kernel Patch Version: %x\n", kpver);

    kf_vmalloc = (typeof(kf_vmalloc))kallsyms_lookup_name("vmalloc");
    if (!kf_vmalloc) {
        pr_info("[yuuki] kernel func: 'vmalloc' does not exist!\n");
        goto exit;
    }

    kf_vfree = (typeof(kf_vfree))kallsyms_lookup_name("vfree");
    if (!kf_vfree) {
        pr_info("[yuuki] kernel func: 'vfree' does not exist!\n");
        goto exit;
    }

    d_path = (typeof(d_path))kallsyms_lookup_name("d_path");
    if (!d_path) {
        pr_info("[yuuki] kernel func: 'd_path' does not exist!\n");
        goto exit;
    }

    fput = (typeof(fput))kallsyms_lookup_name("fput");
    if (!fput) {
        pr_info("[yuuki] kernel func: 'fput' does not exist!\n");
        goto exit;
    }

    exit:
    return ret;
}

static long mod_control0(const char *args, char *__user out_msg, int outlen) {
    pr_info("[yuuki] kpm hello control0, args: %s\n", args);

    parsePaths(args, source_paths, redirect_paths, &line_count);

    for (int i = 0; i < line_count; i++) {
        pr_info("[yuuki] source_path: %s redirect_path: %s\n", source_paths[i], redirect_paths[i]);
    }

    control_internal(true);

    return 0;
}

static long mod_exit(void *__user reserved) {
    control_internal(false);
    pr_info("[yuuki] mod_exit, uninstalled hook.\n");
    return 0;
}

KPM_INIT(mod_init);
KPM_CTL0(mod_control0);
KPM_EXIT(mod_exit);
