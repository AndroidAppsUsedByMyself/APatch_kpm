// https://github.com/SoyBeanMilkx/IO_Redirect/blob/main/src/mod_main.c
#include "data_parse.h"
#include "file.h"
#include <compiler.h>
#include <kpm_hook_utils.h>
#include <kpm_utils.h>
#include <kpmodule.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <syscall.h>
#include <taskext.h>
KPM_NAME("file_ext_rule");
KPM_VERSION("0.0.0");
KPM_AUTHOR("FER");
// We do nothing currently
// KPM_AUTHOR("FER && DataEraserC");
KPM_DESCRIPTION("Kernel-level file rule according to config file");

hook_func_def(do_filp_open, struct file *, int dfd, struct filename *pathname,
              const struct open_flags *op);
hook_func_no_info(do_filp_open);

// Kernel Functions
char *(*kf_d_path)(const struct path *path, char *buf, int buflen) = NULL;
void (*kf_fput)(struct file *file) = NULL;
void *(*kf_vmalloc)(unsigned long size) = NULL;
void (*kf_vfree)(const void *addr) = NULL;

void *kf_do_unlinkat = NULL;
void *kf_do_rmdir = NULL;
void *kf_do_renameat2 = NULL;
void *kf_do_filp_open = NULL;
// TODO: impl my own data structure
// NOTE: do not add lots of rule or it will influence a lot on operating
// efficiency
// can it be possible that only apply rule to some of app?
static char redirect_source_paths[MAX_LINES][PATH_MAX];
static char redirect_target_paths[MAX_LINES][PATH_MAX];
static int redirect_rule_line_count = 0;

static char restrict_do_unlinkat_paths[MAX_LINES][PATH_MAX];
static int restrict_do_unlinkat_rule_line_count = 0;

static char restrict_do_rmdir_paths[MAX_LINES][PATH_MAX];
static int restrict_do_rmdir_rule_line_count = 0;

static char restrict_do_renameat2_source_paths[MAX_LINES][PATH_MAX];
static int restrict_do_renameat2_source_rule_line_count = 0;

static char restrict_do_renameat2_target_paths[MAX_LINES][PATH_MAX];
static int restrict_do_renameat2_target_rule_line_count = 0;

static char restrict_do_filp_open_paths[MAX_LINES][PATH_MAX];
static int restrict_do_filp_open_rule_line_count = 0;

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

// https://github.com/SoyBeanMilkx/FileLock/blob/main/src/module.c#L75-L219
// 删除文件拦截
static void do_unlinkat_before(hook_fargs1_t *args, void *udata) {
  struct filename *pathname = (struct filename *)args->arg1;
  char path_buf[PATH_MAX];

  // 默认放行
  args->skip_origin = false;
  args->ret = 0;

  // 更严格的路径检查
  if (!pathname || IS_ERR(pathname) || !pathname->name) {
    return;
  }

  // 初始化缓冲区
  memset(path_buf, 0, PATH_MAX);
  strncpy(path_buf, pathname->name, PATH_MAX - 1);

  // 检查是否在受保护目录列表中
  for (int i = 0; i < restrict_do_unlinkat_rule_line_count; i++) {
    if (strstr(path_buf, restrict_do_unlinkat_paths[i]) != NULL) {
      pr_info("[FER] do_unlinkat_before: 拦截删除文件操作: %s\n", path_buf);
      args->skip_origin = true;
      args->ret = -EACCES;
      return;
    }
  }

  // 如果没有匹配任何规则，保持默认的跳过行为
  return;
}

// 删除路径拦截
static void do_rmdir_before(hook_fargs2_t *args, void *udata) {
  struct filename *pathname = (struct filename *)args->arg1;
  char path_buf[PATH_MAX];

  // 默认放行
  args->skip_origin = false;
  args->ret = 0;

  // 更严格的路径检查
  if (!pathname || IS_ERR(pathname) || !pathname->name) {
    return;
  }

  // 初始化缓冲区
  memset(path_buf, 0, PATH_MAX);
  strncpy(path_buf, pathname->name, PATH_MAX - 1);

  // 检查是否在受保护目录列表中
  for (int i = 0; i < restrict_do_unlinkat_rule_line_count; i++) {
    if (strstr(path_buf, restrict_do_unlinkat_paths[i]) != NULL) {
      pr_info("[FER] do_rmdir_before: 拦截删除目录操作: %s\n", path_buf);
      args->skip_origin = true;
      args->ret = -EACCES;
      return;
    }
  }

  // 如果没有匹配任何规则，保持默认的跳过行为
  return;
}

// 重命名拦截
static void do_renameat2_before(hook_fargs3_t *args, void *udata) {
  struct filename *oldname = (struct filename *)args->arg1;
  struct filename *newname = (struct filename *)args->arg3;
  char old_path[PATH_MAX];
  char new_path[PATH_MAX];

  // 默认放行
  args->skip_origin = false;
  args->ret = 0;

  // 检查路径有效性
  if (!oldname || IS_ERR(oldname) || !oldname->name || !newname ||
      IS_ERR(newname) || !newname->name) {
    return;
  }

  // 获取源路径和目标路径
  memset(old_path, 0, PATH_MAX);
  memset(new_path, 0, PATH_MAX);
  strncpy(old_path, oldname->name, PATH_MAX - 1);
  strncpy(new_path, newname->name, PATH_MAX - 1);

  // 检查是否在受保护目录列表中
  for (int i = 0; i < restrict_do_renameat2_source_rule_line_count; i++) {
    if (strstr(old_path, restrict_do_renameat2_source_paths[i]) != NULL) {
      pr_info("[FER] do_renameat2_before: 拦截重命名操作: %s -> %s\n", old_path,
              new_path);
      args->skip_origin = true;
      args->ret = -EACCES;
      return;
    }
  }

  // 检查是否在受保护目录列表中
  for (int i = 0; i < restrict_do_renameat2_target_rule_line_count; i++) {
    if (strstr(new_path, restrict_do_renameat2_target_paths[i]) != NULL) {
      pr_info("[FER] do_renameat2_before: 拦截重命名操作: %s -> %s\n", old_path,
              new_path);
      args->skip_origin = true;
      args->ret = -EACCES;
      return;
    }
  }

  // 如果没有匹配任何规则，保持默认的跳过行为
  return;
}

// TODO: impl my own function here
static struct file *hook_replace(do_filp_open)(int dfd,
                                               struct filename *pathname,
                                               const struct open_flags *op) {
  struct file *filp = hook_backup(do_filp_open)(dfd, pathname, op);
  if (likely(!IS_ERR(filp))) {

    char path_buf[PATH_MAX];

    // 初始化缓冲区
    memset(path_buf, 0, PATH_MAX);
    char *currPath = kf_d_path(&filp->f_path, path_buf, PATH_MAX);

    // 对其他路径，只在写入和创建时进行保护
    if ((op->open_flag & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC))) {

      // 检查是否在受保护目录列表中
      for (int i = 0; i < restrict_do_filp_open_rule_line_count; i++) {
        if (strstr(currPath, restrict_do_filp_open_paths[i]) != NULL) {
          pr_info("[FER] do_filp_open_before: 拦截文件操作: %s\n", currPath);
          kf_fput(filp);
          return ERR_PTR(-EACCES);
        }
      }
    }

    for (int i = 0; i < redirect_rule_line_count; i++) {

      if (strncmp(currPath, redirect_source_paths[i],
                  strlen(redirect_source_paths[i])) == 0) {
        pr_info("[FER] Interception path %s successful, redirect to %s\n",
                redirect_source_paths[i], redirect_target_paths[i]);
        kf_fput(filp);

        struct filename *new_pathname = kf_vmalloc(sizeof(struct filename));
        if (!new_pathname) {
          return ERR_PTR(-ENOMEM);
        }
        new_pathname->name = redirect_target_paths[i];

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

static inline bool installHook() {
  bool ret = false;

  // 获取函数地址
  kf_do_unlinkat = (void *)kallsyms_lookup_name("do_unlinkat");
  kf_do_rmdir = (void *)kallsyms_lookup_name("do_rmdir");
  kf_do_renameat2 = (void *)kallsyms_lookup_name("do_renameat2");

  // 安装 wrap hook
  kf_do_unlinkat && hook_wrap1(kf_do_unlinkat, do_unlinkat_before, NULL, NULL);
  kf_do_rmdir && hook_wrap2(kf_do_rmdir, do_rmdir_before, NULL, NULL);
  kf_do_renameat2 && hook_wrap3(kf_do_renameat2, do_renameat2_before, NULL,
                               NULL);
  // 打印 hook 状态，同时考虑函数是否存在
  pr_info("[FER]: do_rmdir : %px", kf_do_rmdir);
  pr_info("[FER]: do_unlinkat : %px", kf_do_unlinkat);
  pr_info("[FER]: do_renameat2 : %px", kf_do_renameat2);

  if (hook_err(do_filp_open) != HOOK_NO_ERR) {
    if (hook_install(do_filp_open)) {
      pr_info("[FER]: do_filp_open : %px", hook_original(do_filp_open));
      ret = true;
    } else {
      pr_info("[FER]: failed to install do_filp_open hook\n");
    }
  } else {
    pr_info("[FER] hook already installed, skipping...\n");
    ret = true;
  }

  return ret;
}

static inline bool uninstallHook() {
  if (kf_do_unlinkat) {
    unhook(kf_do_unlinkat);
  }

  if (kf_do_rmdir) {
    unhook(kf_do_rmdir);
  }

  if (kf_do_renameat2) {
    unhook(kf_do_renameat2);
  }

  memset(redirect_source_paths, 0, sizeof(redirect_source_paths));
  memset(redirect_target_paths, 0, sizeof(redirect_target_paths));
  redirect_rule_line_count = 0;

  memset(restrict_do_unlinkat_paths, 0, sizeof(restrict_do_unlinkat_paths));
  restrict_do_unlinkat_rule_line_count = 0;

  memset(restrict_do_rmdir_paths, 0, sizeof(restrict_do_rmdir_paths));
  restrict_do_rmdir_rule_line_count = 0;

  memset(restrict_do_renameat2_source_paths, 0,
         sizeof(restrict_do_renameat2_source_paths));
  restrict_do_renameat2_source_rule_line_count = 0;

  memset(restrict_do_renameat2_target_paths, 0,
         sizeof(restrict_do_renameat2_target_paths));
  restrict_do_renameat2_target_rule_line_count = 0;

  memset(restrict_do_filp_open_paths, 0, sizeof(restrict_do_filp_open_paths));
  restrict_do_filp_open_rule_line_count = 0;

  pr_info("[FER]: hook 已移除，保护路径已清空\n");
  if (hook_success(do_filp_open)) {
    unhook((void *)hook_original(do_filp_open));
    hook_err(do_filp_open) = HOOK_NOT_HOOK;
    pr_info("[FER]  disbaled !\n");
  } else {
    pr_info("[FER]  Always disabled !\n");
  }
  return true;
}

static inline bool control_internal(bool enable) {
  return enable ? installHook() : uninstallHook();
}

static long mod_init(const char *args, const char *event,
                     void *__user reserved) {
  pr_info("[FER] Initializing...\n");

  hook_original(do_filp_open) =
      (do_filp_open_func_t)kallsyms_lookup_name("do_filp_open");

  long ret = 0;

  pr_info("[FER] Kernel Version: %x\n", kver);
  pr_info("[FER] Kernel Patch Version: %x\n", kpver);

  kf_vmalloc = (typeof(kf_vmalloc))kallsyms_lookup_name("vmalloc");
  if (!kf_vmalloc) {
    pr_info("[FER] kernel func: 'vmalloc' does not exist!\n");
    goto exit;
  }

  kf_vfree = (typeof(kf_vfree))kallsyms_lookup_name("vfree");
  if (!kf_vfree) {
    pr_info("[FER] kernel func: 'vfree' does not exist!\n");
    goto exit;
  }

  kf_d_path = (typeof(kf_d_path))kallsyms_lookup_name("d_path");
  if (!kf_d_path) {
    pr_info("[FER] kernel func: 'd_path' does not exist!\n");
    goto exit;
  }

  kf_fput = (typeof(kf_fput))kallsyms_lookup_name("fput");
  if (!kf_fput) {
    pr_info("[FER] kernel func: 'fput' does not exist!\n");
    goto exit;
  }

exit:
  return ret;
}

static long mod_control0(const char *args, char *__user out_msg, int outlen) {
  pr_info("[FER] kpm hello control0, args: %s\n", args);

  // TODO: impl my own function
  parsePaths(args, redirect_source_paths, redirect_target_paths,
             &redirect_rule_line_count);

  for (int i = 0; i < redirect_rule_line_count; i++) {
    pr_info("[FER] source_path: %s redirect_path: %s\n",
            redirect_source_paths[i], redirect_target_paths[i]);
  }

  control_internal(true);

  return 0;
}

static long mod_exit(void *__user reserved) {
  control_internal(false);
  pr_info("[FER] mod_exit, uninstalled hook.\n");
  return 0;
}

KPM_INIT(mod_init);
KPM_CTL0(mod_control0);
KPM_EXIT(mod_exit);
