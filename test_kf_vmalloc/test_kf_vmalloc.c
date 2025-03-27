
#include <compiler.h>
#include <kallsyms.h>
#include <kpm_utils.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <linux/string.h>
KPM_NAME("test_kf_vmalloc");
KPM_VERSION("0.0.0");
KPM_AUTHOR("DataEraserC");
KPM_DESCRIPTION("Test KF vmalloc");
void *(*kf_vmalloc)(unsigned long size) = NULL;
void (*kf_vfree)(const void *addr) = NULL;

void vmalloc_lookup() {
  kf_vmalloc = (void *)kallsyms_lookup_name("vmalloc");
  if (!kf_vmalloc)
    pr_err("fail to hook vmalloc");
  kf_vfree = (void *)kallsyms_lookup_name("vfree");
  if (!kf_vfree)
    pr_err("fail to hook vfree");
}
static long test_kf_vmalloc_init(const char *args, const char *event,
                                 void *__user reserved) {
  vmalloc_lookup();
  pr_info("kpm test_kf_vmalloc init, event: %s, args: %s\n", event, args);
  pr_info("kernelpatch version: %x\n", kpver);
  return 0;
}

static long test_kf_vmalloc_control0(const char *args, char *__user out_msg,
                                     int outlen) {
  pr_info("kpm test_kf_vmalloc control0, args: %s\n", args);
  char *echo = (char *)kf_vmalloc(64);
  strncat(echo, "echo: ", 7);
  strncat(echo, args, 48);
  writeOutMsg(out_msg, &outlen, echo);
  kf_vfree(echo);
  return 0;
}

static long test_kf_vmalloc_control1(void *a1, void *a2, void *a3) {
  pr_info("kpm test_kf_vmalloc control1, a1: %llx, a2: %llx, a3: %llx\n", a1,
          a2, a3);
  return 0;
}

static long test_kf_vmalloc_exit(void *__user reserved) {
  pr_info("kpm test_kf_vmalloc exit\n");
  return 0;
}

KPM_INIT(test_kf_vmalloc_init);
KPM_CTL0(test_kf_vmalloc_control0);
KPM_CTL1(test_kf_vmalloc_control1);
KPM_EXIT(test_kf_vmalloc_exit);
