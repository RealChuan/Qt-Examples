# PlatformLibraries.pri - 库项目配置

# 包含编译器设置
include(ProjectSettings.pri)

# 设置项目模板为库
TEMPLATE = lib
export(TEMPLATE)

# 设置库输出目录
LIBS += -L$$LIBRARY_OUTPUT_DIRECTORY
export(LIBS)

# 函数：获取小写的项目名称
defineReplace(getProjectNameLower) {
    return($$lower($$PROJECT_NAME))
}

# 函数：为库名称添加调试后缀
defineReplace(replaceLibName) {
    unset(LIBRARY_NAME)
    LIBRARY_NAME = $$1
    CONFIG(debug, debug|release) {
        !debug_and_release|build_pass {
            RET = $$member(LIBRARY_NAME, 0)d
        }
    }
    isEmpty(RET):RET = $$LIBRARY_NAME
    return($$RET)
}

# 函数：配置平台库（静态库）
defineReplace(add_platform_library) {
    LIBRARY_NAME = $$1
    
    win32 {
        DESTDIR = $$LIBRARY_OUTPUT_DIRECTORY
        DLLDESTDIR = $$RUNTIME_OUTPUT_DIRECTORY
        export(DESTDIR)
        export(DLLDESTDIR)
    }
    
    unix {
        # Unix 平台默认使用静态库
        CONFIG += staticlib
        DESTDIR = $$LIBRARY_OUTPUT_DIRECTORY
        export(DESTDIR)
        export(CONFIG)
    }
    
    # 设置目标名称（带调试后缀）
    RET = $$replaceLibName($$LIBRARY_NAME)
    
    message("Platform library configured: $$RET")
    return($$RET)
}

# 函数：配置插件库
defineReplace(add_plugin_library) {
    LIBRARY_NAME = $$1
    
    win32 {
        DESTDIR = $$LIBRARY_OUTPUT_DIRECTORY
        DLLDESTDIR = $$RUNTIME_OUTPUT_DIRECTORY/plugins/$$getProjectNameLower()
        export(DESTDIR)
        export(DLLDESTDIR)
    }
    
    macx {
        DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY/$$member(PROJECT_NAME, 0).app/Contents/PlugIns/$$getProjectNameLower()
        export(DESTDIR)
    }
    
    unix:!macx {
        DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY/plugins/$$getProjectNameLower()
        export(DESTDIR)
    }
    
    # 设置目标名称（带调试后缀）
    RET = $$replaceLibName($$LIBRARY_NAME)
    
    message("Plugin library configured: $$RET")
    return($$RET)
}

# 函数：配置共享库
defineReplace(add_shared_library) {
    LIBRARY_NAME = $$1
    
    win32 {
        DESTDIR = $$LIBRARY_OUTPUT_DIRECTORY
        DLLDESTDIR = $$RUNTIME_OUTPUT_DIRECTORY
        export(DESTDIR)
        export(DLLDESTDIR)
    }
    
    unix {
        DESTDIR = $$LIBRARY_OUTPUT_DIRECTORY
        export(DESTDIR)
    }
    
    # 设置目标名称（带调试后缀）
    RET = $$replaceLibName($$LIBRARY_NAME)
    
    message("Shared library configured: $$RET")
    return($$RET)
}
