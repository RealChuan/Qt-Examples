# ArchitectureDetection.pri - 架构检测函数

# 函数：验证架构字符串是否有效
defineTest(is_valid_architecture) {
    arch = $$1
    contains(arch, "x86_64") | contains(arch, "x64") | contains(arch, "amd64") | \
    contains(arch, "arm64") | contains(arch, "aarch64") | \
    contains(arch, "i386") | contains(arch, "x86") | \
    contains(arch, "universal") {
        return(true)
    }
    return(false)
}

# 函数：规范化架构名称
defineReplace(normalize_arch_name) {
    arch = $$1
    contains(arch, "x86_64") | contains(arch, "x64") | contains(arch, "amd64") {
        return(x86_64)
    }
    contains(arch, "arm64") | contains(arch, "aarch64") {
        return(arm64)
    }
    contains(arch, "i386") | contains(arch, "x86") {
        return(x86)
    }
    contains(arch, "universal") {
        return(universal)
    }
    return(unknown)
}

# 函数：检查列表是否包含特定架构
defineTest(contains_architecture) {
    arch_list = $$1
    target_arch = $$2
    
    for(arch, arch_list) {
        equals(arch, $$target_arch) {
            return(true)
        }
    }
    return(false)
}

# 函数：检查列表是否同时包含 x86_64 和 arm64
defineTest(contains_both_x64_and_arm64) {
    arch_list = $$1
    
    has_x64 = false
    has_arm64 = false
    
    for(arch, arch_list) {
        equals(arch, "x86_64") {
            has_x64 = true
        }
        equals(arch, "arm64") {
            has_arm64 = true
        }
    }
    
    equals(has_x64, "true"):equals(has_arm64, "true") {
        return(true)
    }
    return(false)
}

# 函数：检测当前构建的架构
defineReplace(detect_architecture) {
    # 1. 优先检测 macOS 通用二进制（用户显式指定）
    macx {
        !isEmpty(QMAKE_APPLE_DEVICE_ARCHS) {
            valid_arch_count = 0
            supported_archs =

            for(arch, QMAKE_APPLE_DEVICE_ARCHS) {
                normalized_arch = $$normalize_arch_name($$arch)
                is_valid_architecture($$arch) {
                    valid_arch_count = $$val(valid_arch_count) + 1
                    supported_archs = $$supported_archs $$normalized_arch
                } else {
                    message("Warning: Unsupported architecture '$$arch' in QMAKE_APPLE_DEVICE_ARCHS")
                }
            }

            equals(valid_arch_count, 0) {
                error("No valid architectures found in QMAKE_APPLE_DEVICE_ARCHS: $$QMAKE_APPLE_DEVICE_ARCHS")
                return(unknown)
            } else:equals(valid_arch_count, 1) {
                return($$first(supported_archs))
            } else {
                contains_both_x64_and_arm64($$supported_archs) {
                    message("Detected universal binary (x86_64 + arm64)")
                    return(universal)
                } else {
                    message("Multiple architectures detected: $$supported_archs, using first")
                    return($$first(supported_archs))
                }
            }
        }
    }

    # 2. 其次使用 uname -m 获取真实硬件架构（绕过 Rosetta）
    macx {
        native_arch = $$system(uname -m)
        normalized_native = $$normalize_arch_name($$native_arch)
        is_valid_architecture($$normalized_native) {
            return($$normalized_native)
        }
    }

    # 3. 检测 QT_ARCH（跨平台 Qt 架构变量）
    !isEmpty(QT_ARCH) {
        normalized_qt_arch = $$normalize_arch_name($$QT_ARCH)
        is_valid_architecture($$normalized_qt_arch) {
            return($$normalized_qt_arch)
        }
    }

    # 4. 使用 QMAKE_HOST.arch 进行最终检测
    !isEmpty(QMAKE_HOST.arch) {
        normalized_host_arch = $$normalize_arch_name($$QMAKE_HOST.arch)
        is_valid_architecture($$normalized_host_arch) {
            return($$normalized_host_arch)
        }
    }

    # 默认情况
    message("Warning: Unable to detect architecture, using 'unknown'")
    return(unknown)
}

defineReplace(get_standard_architecture) {
    arch = $$1
    
    # 首先处理通用二进制
    equals(arch, "universal") {
        return(universal)
    }
    
    # 处理标准架构命名
    equals(arch, "x86_64") {
        return(x64)
    }
    equals(arch, "arm64") {
        return(arm64)
    }
    equals(arch, "x86") {
        return(x86)
    }
    
    # 未知架构
    return(unknown)
}

TARGET_ARCH = $$detect_architecture()
TARGET_ARCH = $$get_standard_architecture($$TARGET_ARCH)
