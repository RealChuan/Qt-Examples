PROJECT_NAME = Qt-Examples

# -----------------------------------------------------------------------------
# 版本号唯一来源：packaging/version.txt（与 CMake 侧读同一个文件）
#   允许命令行覆盖：qmake VERSION=1.2.3
# -----------------------------------------------------------------------------
isEmpty(VERSION) {
    VERSION_LINES = $$cat($$PWD/../packaging/version.txt, lines)
    VERSION = $$first(VERSION_LINES)
    isEmpty(VERSION): error("无法从 packaging/version.txt 读取版本号")
    # Windows 资源文件需要 4 段版本号
    VERSION = $${VERSION}.0
}

isEmpty(RC_LANG): RC_LANG = 0x0004
CONFIG += skip_target_version_ext

isEmpty(QMAKE_TARGET_COMPANY): QMAKE_TARGET_COMPANY = The Youth.
isEmpty(QMAKE_TARGET_DESCRIPTION): QMAKE_TARGET_DESCRIPTION = $$PROJECT_NAME
isEmpty(QMAKE_TARGET_COPYRIGHT): QMAKE_TARGET_COPYRIGHT = Copyright (C) 2017-2026 Youth.
isEmpty(QMAKE_TARGET_PRODUCT): QMAKE_TARGET_PRODUCT = $$QMAKE_TARGET_DESCRIPTION
isEmpty(QMAKE_TARGET_ORIGINAL_FILENAME): QMAKE_TARGET_ORIGINAL_FILENAME = $$QMAKE_TARGET_DESCRIPTION
isEmpty(QMAKE_TARGET_INTERNALNAME): QMAKE_TARGET_INTERNALNAME = $$QMAKE_TARGET_DESCRIPTION
isEmpty(QMAKE_TARGET_COMMENTS): QMAKE_TARGET_COMMENTS = $$QMAKE_TARGET_DESCRIPTION
isEmpty(QMAKE_TARGET_TRADEMARKS): QMAKE_TARGET_TRADEMARKS = Youth

include(CompilerSettings.pri)
include(ArchitectureDetection.pri)
include(OutputDirectories.pri)
