# =============================================================================
# PlatformLibraries.pri - 库项目配置
#
# 与 cmake/PlatformLibraries.cmake 对应：
#   - 平台决定库类型（Windows 动态库 / 其他平台静态库）
#   - 导出宏在这里统一写入，各 .pro 不再重复 DEFINES += X_LIBRARY
#
# 关于 export()：qmake 的 defineReplace 函数体有独立变量作用域，函数内的
# DEFINES/DESTDIR/CONFIG 赋值默认不会影响调用者，必须 export() 才能生效。
# 这不是冗余调用，删掉会导致 DESTDIR 失效、导出宏丢失。
# =============================================================================

include(ProjectSettings.pri)

TEMPLATE = lib
LIBS += -L$$LIBRARY_OUTPUT_DIRECTORY

# 函数：获取小写的项目名称
defineReplace(getProjectNameLower) {
    return($$lower($$PROJECT_NAME))
}

# 函数：为库名称添加调试后缀
#   debug_and_release 的 meta 轮次只生成 Makefile，不加后缀
defineReplace(replaceLibName) {
    RET = $$1
    CONFIG(debug, debug|release) {
        !debug_and_release|build_pass: RET = $${RET}d
    }
    return($$RET)
}

# 函数：配置平台库（Windows 动态库 / 其他平台静态库）
defineReplace(add_platform_library) {
    LIBRARY_NAME = $$1
    LIB_UPPER = $$upper($$LIBRARY_NAME)

    # 与 CMake 侧一致：Windows 动态库用 <TARGET>_LIBRARY，其他平台静态库用
    # <TARGET>_STATIC_LIBRARY。放在作用域外，保证任何平台都一定写入宏。
    win32 {
        DEFINES += $${LIB_UPPER}_LIBRARY
    } else {
        DEFINES += $${LIB_UPPER}_STATIC_LIBRARY
    }
    export(DEFINES)

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
        export(CONFIG)
        export(DESTDIR)
    }

    RET = $$replaceLibName($$LIBRARY_NAME)
    !isEmpty(VERBOSE): message("Platform library: $$RET")
    return($$RET)
}

# 函数：配置插件库（与 CMake 的 add_app_plugin 对应）
#   CONFIG += plugin 让 qmake 产出可被 QPluginLoader 加载的插件，而不是普通共享库。
defineReplace(add_app_plugin) {
    LIBRARY_NAME = $$1
    LIB_UPPER = $$upper($$LIBRARY_NAME)

    DEFINES += $${LIB_UPPER}_LIBRARY
    CONFIG += plugin
    export(DEFINES)
    export(CONFIG)

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

    RET = $$replaceLibName($$LIBRARY_NAME)
    !isEmpty(VERBOSE): message("App plugin: $$RET")
    return($$RET)
}

# 函数：配置共享库
defineReplace(add_shared_library) {
    LIBRARY_NAME = $$1
    LIB_UPPER = $$upper($$LIBRARY_NAME)

    DEFINES += $${LIB_UPPER}_LIBRARY
    export(DEFINES)

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

    RET = $$replaceLibName($$LIBRARY_NAME)
    !isEmpty(VERBOSE): message("Shared library: $$RET")
    return($$RET)
}
