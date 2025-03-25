#include "file.h"
#include <compiler.h>
#include <kpmodule.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <syscall.h>
#include <taskext.h>
KPM_NAME("file_ext_rule");
KPM_VERSION("0.0.0");
KPM_AUTHOR("yuuki && DataEraserC");
KPM_DESCRIPTION("Kernel-level file rule according to config file");

static inline void set_priv_selinx_allow(struct task_struct *task, int val);
typedef struct file *(*do_filp_open_func_t)(int dfd, struct filename *pathname,
                                            const struct open_flags *op);
static do_filp_open_func_t original_do_filp_open = NULL;
static do_filp_open_func_t backup_do_filp_open = NULL;
static struct file *replace_do_filp_open(int dfd, struct filename *pathname,
                                         const struct open_flags *op);

static hook_err_t hook_err = HOOK_NOT_HOOK;

char *(*d_path)(const struct path *path, char *buf, int buflen) = NULL;
void (*fput)(struct file *file) = NULL;

void *(*kf_vmalloc)(unsigned long size) = NULL;
void (*kf_vfree)(const void *addr) = NULL;

KPM_INIT(mod_init);
KPM_CTL0(mod_control0);
KPM_EXIT(mod_exit);
