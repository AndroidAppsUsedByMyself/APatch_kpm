#ifndef _KPM_UTILS_H
#define _KPM_UTILS_H

#include <asm/current.h>
#include <taskext.h>
#include <kpmodule.h>
#include <kputils.h>
#include <linux/cred.h>
#include <linux/printk.h>
#include <linux/sched.h>

#include "sk_func.h"

static inline void get_current_ids(pid_t *pid, pid_t *tgid)
{
    struct task_ext *ext = current_ext; //current_ext
    if (likely(task_ext_valid(ext))) {
        *pid = ext->pid;
        *tgid = ext->tgid;
    }
}

static inline void set_priv_selinx_allow(struct task_struct* task, int val) {
  struct task_ext* ext = get_task_ext(task);
  if (likely(task_ext_valid(ext))) {
    ext->priv_sel_allow = val;
    dsb(ish);
  }
}


static inline void writeOutMsg(char *__user out_msg, int *outlen, const char *msg)
{
    *outlen = strlen(msg);
    compat_copy_to_user(out_msg, msg, *outlen);
}

//  __task_pid_nr_ns
pid_t skfunc_def(__task_pid_nr_ns)(struct task_struct *task, enum pid_type type, struct pid_namespace *ns) = NULL;
static inline pid_t ___task_pid_nr_ns(struct task_struct *task, enum pid_type type, struct pid_namespace *ns)
{
    return skfunc(__task_pid_nr_ns)(task, type, ns);
}
// end

static inline bool initializationKpmFuncs()
{
    bool ret = false;

    skfunc_match(__task_pid_nr_ns, NULL, NULL);
    if (!skfunc(__task_pid_nr_ns)) goto exit;
    pr_info("KPM: __task_pid_nr_ns addr: %llx\n", skfunc(__task_pid_nr_ns));

    ret = true;
exit:
    return ret;
}

#endif /* _KPM_UTILS_H */
