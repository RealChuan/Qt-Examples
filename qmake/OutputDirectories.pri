# OutputDirectories.pri - 输出目录配置

# 使用 $$PWD 明确路径基准，避免相对路径歧义
CONFIG(debug, debug|release) {
    RUNTIME_OUTPUT_DIRECTORY = $$PWD/../output/$$TARGET_ARCH/bin/debug
    LIBRARY_OUTPUT_DIRECTORY = $$PWD/../output/$$TARGET_ARCH/lib/debug
} else {
    RUNTIME_OUTPUT_DIRECTORY = $$PWD/../output/$$TARGET_ARCH/bin/release
    LIBRARY_OUTPUT_DIRECTORY = $$PWD/../output/$$TARGET_ARCH/lib/release
}

message("RUNTIME_OUTPUT_DIRECTORY: $$RUNTIME_OUTPUT_DIRECTORY")
message("LIBRARY_OUTPUT_DIRECTORY: $$LIBRARY_OUTPUT_DIRECTORY")

INCLUDEPATH += $$PWD/../src/
DEPENDPATH  += $$PWD/../src/
