#ifndef _KPM_UTILS_MISC_
#define _KPM_UTILS_MISC_
static inline void printInfo() {
  pr_info("Kernel Version: %x\n", kver);
  pr_info("Kernel Patch Version: %x\n", kpver);
}
#endif
