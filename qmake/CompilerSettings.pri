# =============================================================================
# Compiler Settings（与 cmake/CompilerSettings.cmake 对齐）
# =============================================================================

macx {
    # 允许用环境变量覆盖，默认与 CMake 的 macOSDeploymentTarget.cmake 一致
    QMAKE_MACOSX_DEPLOYMENT_TARGET = $$(MACOSX_DEPLOYMENT_TARGET)
    isEmpty(QMAKE_MACOSX_DEPLOYMENT_TARGET): QMAKE_MACOSX_DEPLOYMENT_TARGET = 14.4
}

CONFIG += c++23 strict_c++

# Qt 弃用控制（与 cmake/QtSetup.cmake 完全一致）
#   DISABLE = 6.5.0  : 以 Qt 6.5 LTS 为基线，更早弃用的 API 直接编译报错
#   WARN    = 6.12.0 : 告警上限对齐实际构建版本
DEFINES += \
    QT_DISABLE_DEPRECATED_UP_TO=0x060500 \
    QT_WARN_DEPRECATED_UP_TO=0x060C00

# Release 模式携带调试信息（对应 CMake 的 RELEASE_WITH_DEBUGINFO）
QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE += $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# 编译器警告
gcc|clang {
    QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic
}

msvc {
    # /utf-8         : 源码含中文注释，缺省代码页会把 UTF-8 当本地代码页解析
    # /Zc:__cplusplus: 让 __cplusplus 反映真实的 -std:c++latest
    QMAKE_CXXFLAGS += /W3 /permissive- /utf-8 /Zc:__cplusplus
    # /MP：jom/nmake 不会自动并行；如已用并行构建器可 qmake NO_MP=1 关掉
    !contains(NO_MP, 1): QMAKE_CXXFLAGS += /MP
}

# -----------------------------------------------------------------------------
# ccache（仅 unix，Windows 下 cl.exe 建议改用 CMake + sccache）
# -----------------------------------------------------------------------------
unix {
    CCACHE_BIN = $$system(which ccache 2>/dev/null)
    !isEmpty(CCACHE_BIN) {
        !contains(USE_CCACHE, 0) {
            QMAKE_CXX = $$CCACHE_BIN $$QMAKE_CXX
            QMAKE_CC = $$CCACHE_BIN $$QMAKE_CC
            !isEmpty(VERBOSE): message("ccache enabled: $$CCACHE_BIN")
        }
    }
}
