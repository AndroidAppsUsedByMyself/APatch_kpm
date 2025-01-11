/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 bmax121. All Rights Reserved.
 * Copyright (C) 2024 lzghzr. All Rights Reserved.
 */
#ifndef __module_check_version_ignore_H
#define __module_check_version_ignore_H

#include <hook.h>
#include <ksyms.h>
#include <linux/cred.h>
#include <linux/sched.h>

#define lookup_name(func)                                                      \
  func = 0;                                                                    \
  func = (typeof(func))kallsyms_lookup_name(#func);                            \
  pr_info("kernel function %s addr: %llx\n", #func, func);                     \
  if (!func) {                                                                 \
    pr_err("lookup %s failed: make sure symbol %s is exported\n", #func,       \
           #func);                                                             \
    return -21;                                                                \
  }

#define hook_func(func, argv, before, after, udata)                            \
  if (!func) {                                                                 \
    return -22;                                                                \
  }                                                                            \
  hook_err_t hook_err_##func = hook_wrap(func, argv, before, after, udata);    \
  if (hook_err_##func) {                                                       \
    func = 0;                                                                  \
    pr_err("hook %s error: %d\n", #func, hook_err_##func);                     \
    return -23;                                                                \
  } else {                                                                     \
    pr_info("hook %s success\n", #func);                                       \
  }

#define unhook_func(func)                                                      \
  if (func && !is_bad_address(func)) {                                         \
    unhook(func);                                                              \
    func = 0;                                                                  \
  }

#endif /* __module_check_version_ignore_H */
