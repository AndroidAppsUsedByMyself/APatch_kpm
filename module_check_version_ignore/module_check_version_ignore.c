/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 bmax121. All Rights Reserved.
 * Copyright (C) 2024 lzghzr. All Rights Reserved.
 */

#include <compiler.h>
#include <hook.h>
#include <kpmodule.h>
#include <kputils.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <taskext.h>
#include <uapi/asm-generic/errno.h>

#include "module_check_version_ignore.h"

KPM_NAME("module_check_version_ignore");
KPM_VERSION(MYKPM_VERSION);
KPM_LICENSE("GPL v2");
KPM_AUTHOR("DataEraserC");
KPM_DESCRIPTION("module_check_version_ignore");

// hook check_version
static int (*check_version)(const struct load_info *info, const char *symname,
                            struct module *mod, const s32 *crc);

static void check_version_after(hook_fargs4_t *args, void *udata) {
  pr_info("after add check_version\n");
}
static void check_version_before(hook_fargs4_t *args, void *udata) {
  pr_info("before add check_version\n");
  if (!(int)args->ret) {
    pr_warn("the return value of check_version is 0, try to bypass by set it "
            "to 1\n");
    args->ret = 1;
  }
}

static long inline_hook_init(const char *args, const char *event,
                             void *__user reserved) {
  lookup_name(check_version);
  hook_func(check_version, 4, check_version_before, check_version_after, 0);
  return 0;
}

static long inline_hook_control0(const char *args, char *__user out_msg,
                                 int outlen) {
  pr_info("check_version_hook control, args: %s\n", args);
  return 0;
}

static long inline_hook_exit(void *__user reserved) {
  unhook_func(check_version);

  return 0;
}

KPM_INIT(inline_hook_init);
KPM_CTL0(inline_hook_control0);
KPM_EXIT(inline_hook_exit);
