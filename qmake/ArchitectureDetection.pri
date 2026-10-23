# =============================================================================
# ArchitectureDetection.pri - 目标架构检测（与 cmake/ArchitectureDetection.cmake 对齐）
#
#   TARGET_ARCH : 统一命名 x86_64 / arm64 / x86 / universal / unknown
#   ARCH_DIR    : 目录名   x64 / arm64 / x86 / universal / unknown
#
# 本文件被 ProjectSettings.pri 与 VcpkgToolchain.pri 重复 include，
# 用 isEmpty() 守卫保证幂等（qmake 的 include 没有内置防重入）。
# =============================================================================

# 把各种写法归一成内部命名；无法识别时返回空串
defineReplace(normalize_arch_name) {
    arch = $$1

    contains(arch, "x86_64")|contains(arch, "x64")|contains(arch, "amd64")|contains(arch, "AMD64") {
        return(x86_64)
    }
    contains(arch, "arm64")|contains(arch, "aarch64")|contains(arch, "ARM64") {
        return(arm64)
    }
    contains(arch, "i386")|contains(arch, "x86")|contains(arch, "Win32") {
        return(x86)
    }
    contains(arch, "universal") {
        return(universal)
    }

    return()
}

# 目录名 / vcpkg triplet 前缀
defineReplace(arch_dir_name) {
    arch = $$1

    equals(arch, "x86_64"): return(x64)
    equals(arch, "arm64"): return(arm64)
    equals(arch, "x86"): return(x86)
    equals(arch, "universal"): return(universal)

    return(unknown)
}

defineReplace(detect_architecture) {
    # 1. macOS：显式指定的架构 / 通用二进制
    macx {
        !isEmpty(QMAKE_APPLE_DEVICE_ARCHS) {
            archs =
            for(a, QMAKE_APPLE_DEVICE_ARCHS) {
                n = $$normalize_arch_name($$a)
                !isEmpty(n): archs += $$n
            }

            !isEmpty(archs) {
                contains(archs, "x86_64"):contains(archs, "arm64"): return(universal)
                return($$first(archs))
            }
        }

        # 2. uname -m 拿到真实硬件架构（可绕过 Rosetta）
        native = $$system(uname -m)
        n = $$normalize_arch_name($$native)
        !isEmpty(n): return($$n)
    }

    # 3. 交叉编译时 mkspec 里的目标架构
    !isEmpty(QT_ARCH) {
        n = $$normalize_arch_name($$QT_ARCH)
        !isEmpty(n): return($$n)
    }

    # 4. 主机架构兜底
    !isEmpty(QMAKE_HOST.arch) {
        n = $$normalize_arch_name($$QMAKE_HOST.arch)
        !isEmpty(n): return($$n)
    }

    return(unknown)
}

# 允许命令行覆盖：qmake TARGET_ARCH=arm64
isEmpty(TARGET_ARCH) {
    TARGET_ARCH = $$detect_architecture()
} else {
    TARGET_ARCH_NORMALIZED = $$normalize_arch_name($$TARGET_ARCH)
    !isEmpty(TARGET_ARCH_NORMALIZED): TARGET_ARCH = $$TARGET_ARCH_NORMALIZED
}

isEmpty(ARCH_DIR): ARCH_DIR = $$arch_dir_name($$TARGET_ARCH)

!isEmpty(VERBOSE): message("Target architecture: $$TARGET_ARCH (dir: $$ARCH_DIR)")
