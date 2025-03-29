#include <compiler.h>
#include <kallsyms.h>
#include <kpm_utils.h>
#include <kpmodule.h>
#include <kpmalloc.h>
#include <linux/printk.h>
#include <linux/string.h>
KPM_NAME("test_kp_malloc");
KPM_VERSION("0.0.0");
KPM_AUTHOR("DataEraserC");
KPM_DESCRIPTION("Test KF vmalloc");

static long test_kp_malloc_init(const char *args, const char *event,
                                 void *__user reserved) {
  pr_info("kpm test_kp_malloc init, event: %s, args: %s\n", event, args);
  pr_info("kernelpatch version: %x\n", kpver);
  return 0;
}

static long test_kp_malloc_control0(const char *args, char *__user out_msg,
                                     int outlen) {
  pr_info("kpm test_kp_malloc control0, args: %s\n", args);
  char *echo = (char *)kp_malloc(64);
  strncat(echo, "echo: ", 7);
  strncat(echo, args, 48);
  writeOutMsg(out_msg, &outlen, echo);
  kp_free(echo);
  return 0;
}

static long test_kp_malloc_control1(void *a1, void *a2, void *a3) {
  pr_info("kpm test_kp_malloc control1, a1: %llx, a2: %llx, a3: %llx\n", a1,
          a2, a3);
  return 0;
}

static long test_kp_malloc_exit(void *__user reserved) {
  pr_info("kpm test_kp_malloc exit\n");
  return 0;
}

KPM_INIT(test_kp_malloc_init);
KPM_CTL0(test_kp_malloc_control0);
KPM_CTL1(test_kp_malloc_control1);
KPM_EXIT(test_kp_malloc_exit);
