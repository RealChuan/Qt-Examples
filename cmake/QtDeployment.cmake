# cmake-format: off
# Qt 部署：翻译产出 + 部署脚本
#
#   本文件只放"把 Qt 应用交付出去"相关的两件事：
#     1. add_translations()      —— .ts/.qm 的构建与安装
#     2. deploy_qt_application()  —— 调用 Qt 的部署工具（windeployqt/macdeployqt）
# cmake-format: on

# -----------------------------------------------------------------------------
# 自动配置检查
# -----------------------------------------------------------------------------
function(check_qt_translations_setup)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/translations")
    message(
      STATUS
        "Creating translations directory: ${PROJECT_SOURCE_DIR}/translations")
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/translations")
  endif()
endfunction()

# 自动运行配置检查
check_qt_translations_setup()

# cmake-format: off
# 添加Qt翻译支持
#   .qm 必须落在可执行文件旁（LanguageManager 用 applicationDirPath()/translations），
#   多配置生成器下因此要带上 <CONFIG> 子目录。
#   输出目录由 qt_add_translations 在构建期自行创建，这里不预先 file(MAKE_DIRECTORY)。
# cmake-format: on
function(add_translations TARGET_NAME)
  # 参数验证
  if(NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "Target '${TARGET_NAME}' does not exist")
  endif()

  # 转换目标名称为小写用于翻译文件基名
  string(TOLOWER "${TARGET_NAME}" TS_FILE_BASE)

  if(APPLE)
    set(TRANSLATIONS_OUTPUT_DIR
        "${PROJECT_RUNTIME_BASE}/${TARGET_NAME}.app/Contents/Resources/translations"
    )
  else()
    set(TRANSLATIONS_OUTPUT_DIR "${PROJECT_RUNTIME_BASE}/translations")
  endif()

  qt_add_translations(
    ${TARGET_NAME}
    TS_FILE_BASE
    ${TS_FILE_BASE}
    TS_OUTPUT_DIRECTORY
    "${PROJECT_SOURCE_DIR}/translations"
    QM_OUTPUT_DIRECTORY
    "${TRANSLATIONS_OUTPUT_DIR}"
    INCLUDE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/src"
    LUPDATE_OPTIONS
    -no-obsolete
    LRELEASE_OPTIONS
    -compress)

  install(
    DIRECTORY "${TRANSLATIONS_OUTPUT_DIR}/"
    DESTINATION "${TRANSLATION_INSTALL_DIR}"
    FILES_MATCHING
    PATTERN "*.qm")

  # 实际参与构建的 ts 文件（TS_FILE_BASE 模式下由 Qt 按语言生成路径）
  file(GLOB _ts_files "${PROJECT_SOURCE_DIR}/translations/${TS_FILE_BASE}_*.ts")
  message(STATUS "Translations for ${TARGET_NAME}:")
  message(STATUS "  TS files: ${_ts_files}")
  message(STATUS "  Output directory: ${TRANSLATIONS_OUTPUT_DIR}")
  message(STATUS "  Install directory: ${TRANSLATION_INSTALL_DIR}")
endfunction()

# =============================================================================
# 部署Qt应用
# =============================================================================
function(deploy_qt_application TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "deploy_qt_application: 目标不存在: ${TARGET}")
  endif()

  set(_deploy_tool_options "")

  if(WIN32)
    # 部署 MSVC 运行时
    list(APPEND _deploy_tool_options --compiler-runtime)

    # --openssl-root 必须指向「含 DLL 的动态 triplet」： 本项目链接用的是
    # *-windows-static-md（静态库，只有 .lib）， 而 Qt 是运行时才动态加载 OpenSSL 的，windeployqt
    # 在静态目录里 找不到 DLL 会直接以非 0 退出，把 cmake --install 拖挂。
    set(_openssl_triplet "${VCPKG_TARGET_TRIPLET}")
    if(_openssl_triplet MATCHES "^(.+)-windows-(static-md|static)$")
      set(_openssl_triplet "${CMAKE_MATCH_1}-windows")
    elseif(NOT _openssl_triplet)
      set(_openssl_triplet "${ARCH_DIR}-windows")
    endif()

    # 缓存里如果是旧的静态目录（没有 bin/），丢弃重探
    if(WINDEPLOYQT_OPENSSL_ROOT AND NOT EXISTS "${WINDEPLOYQT_OPENSSL_ROOT}/bin")
      message(
        STATUS
          "windeployqt: 忽略没有 DLL 的 WINDEPLOYQT_OPENSSL_ROOT=${WINDEPLOYQT_OPENSSL_ROOT}"
      )
      unset(WINDEPLOYQT_OPENSSL_ROOT CACHE)
    endif()

    if(NOT WINDEPLOYQT_OPENSSL_ROOT)
      # manifest 模式装到 <build>/vcpkg_installed/<triplet>， 经典模式装到
      # <vcpkg>/installed/<triplet>，两种都尝试一次。
      foreach(_candidate "${VCPKG_INSTALLED_DIR}/${_openssl_triplet}"
                         "${VCPKG_DEFAULT_ROOT}/installed/${_openssl_triplet}")
        if(EXISTS "${_candidate}/bin")
          set(WINDEPLOYQT_OPENSSL_ROOT
              "${_candidate}"
              CACHE PATH "windeployqt --openssl-root")
          break()
        endif()
      endforeach()
    endif()

    if(WINDEPLOYQT_OPENSSL_ROOT)
      message(STATUS "windeployqt: --openssl-root ${WINDEPLOYQT_OPENSSL_ROOT}")
      list(APPEND _deploy_tool_options --openssl-root
           "${WINDEPLOYQT_OPENSSL_ROOT}")
    else()
      message(
        WARNING
          "windeployqt: 未找到含 DLL 的 OpenSSL 目录（triplet=${_openssl_triplet}），"
          "将不部署 OpenSSL，目标机上 HTTPS 可能不可用")
    endif()
  endif()

  # 生成部署脚本
  qt_generate_deploy_app_script(TARGET ${TARGET} OUTPUT_SCRIPT _deploy_script
                                DEPLOY_TOOL_OPTIONS ${_deploy_tool_options})

  # 安装部署脚本
  install(SCRIPT ${_deploy_script})
endfunction()
