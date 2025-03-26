add_rules("mode.debug", "mode.release")

target("file_ext_rule")
    set_kind("binary")
    add_files("file_ext_rule/file_ext_rule.c")
    add_includedirs("file_ext_rule")
    add_includedirs("Utils")
    add_includedirs("KernelPatch/kernel")
    add_includedirs("KernelPatch/kernel/include")
    add_includedirs("KernelPatch/kernel/patch/include")
    add_includedirs("KernelPatch/kernel/linux/include")
    add_includedirs("KernelPatch/kernel/linux/arch/arm64/include")
    add_includedirs("KernelPatch/kernel/linux/tools/arch/arm64/include")
    add_defines("MYKPM_VERSION= \"1.0.0\"")

    -- 设置 debug 模式的策略
    set_policy("build.optimization", "none") -- 关闭优化
    set_policy("build.symbols", "debug") -- 开启调试符号

    -- 设置 release 模式的策略
    set_policy("build.optimization", "full") -- 开启全优化
    set_policy("build.symbols", "release") -- 关闭调试符号

-- target("module_check_version_ignore")
--     set_kind("binary")
--     add_files("module_check_version_ignore/module_check_version_ignore.c")
--     add_includedirs("module_check_version_ignore")
--     add_includedirs("Utils")
--     add_includedirs("KernelPatch/kernel")
--     add_includedirs("KernelPatch/kernel/include")
--     add_includedirs("KernelPatch/kernel/patch/include")
--     add_includedirs("KernelPatch/kernel/linux/include")
--     add_includedirs("KernelPatch/kernel/linux/arch/arm64/include")
--     add_includedirs("KernelPatch/kernel/linux/tools/arch/arm64/include")
--     add_defines("MYKPM_VERSION= \"1.0.0\"")

--     -- 设置 debug 模式的策略
--     set_policy("build.optimization", "none") -- 关闭优化
--     set_policy("build.symbols", "debug") -- 开启调试符号

--     -- 设置 release 模式的策略
--     set_policy("build.optimization", "full") -- 开启全优化
--     set_policy("build.symbols", "release") -- 关闭调试符号