# cgroupv2_freeze
## 模块作用
为低版本内核添加 cgroup.freeze

## 更新记录
### 1.0.11
适配更多内核<br />
优化挂载脚本
### 1.0.10
优化 `MIUI` 内核
### 1.0.9
v2 检测函数由 `do_freezer_trap` 改为 `cgroup_freeze_write`
### 1.0.8
修复冻结失效
### 1.0.7
临时 hook `call_usermodehelper_exec`
### 1.0.6
为高版本内核禁用 `CONFIG_STATIC_USERMODEHELPER`
### 1.0.5
补全uid模式<br />
不再需要附加模块
### 1.0.4
使用其他方式替代所有内核版本判断条件<br />
变更 task_struct->jobctl 获取方式
### 1.0.3
支持 4.19
### 1.0.2
支持 4.9
### 1.0.1
不再修改内核配置文件
