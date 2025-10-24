# InstallCrashpad.pri - 安装 Crashpad 处理器

include(VcpkgToolchain.pri)

defineReplace(setup_crashpad_handler) {
    # 获取参数
    target_dir = $$1
    
    commands = 
    
    win32 {
        src_path = $$vcpkg_path/tools/crashpad
        dest_path = $$target_dir
        commands += $$QMAKE_COPY_DIR $$replace(src_path, /, \\) $$replace(dest_path, /, \\)
    }
  
    unix {
        src_path = "$$vcpkg_path/tools/crashpad/*"
        dest_path = $$target_dir
        commands += $$QMAKE_COPY_DIR "$$src_path" "$$dest_path"
        commands += && chmod +x "$$dest_path/crashpad_handler"
    }
    
    return($$commands)
}
