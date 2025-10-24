# =============================================================================
# Qt Utilities - Qt实用工具集
# =============================================================================

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

# -----------------------------------------------------------------------------
# 添加Qt翻译支持
# -----------------------------------------------------------------------------
function(add_translations TARGET_NAME)
  # 参数验证
  if(NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "Target '${TARGET_NAME}' does not exist")
  endif()

  # 转换目标名称为小写用于翻译文件基名
  string(TOLOWER "${TARGET_NAME}" TS_FILE_BASE)

  # 确定翻译文件的输出位置
  if(CMAKE_HOST_APPLE)
    # macOS: 放入App Bundle的Resources目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.app/Contents/Resources/translations"
    )
  elseif(CMAKE_HOST_WIN32)
    # Windows: 放入translations目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations")
  else()
    # Linux/Unix: 放入translations目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations")
  endif()

  # 创建输出目录
  file(MAKE_DIRECTORY "${TRANSLATIONS_OUTPUT_DIR}")

  # 添加Qt翻译
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

  # 显示配置信息
  message(STATUS "Translations for ${TARGET_NAME}:")
  message(STATUS "  TS files: ${TS_FILES}")
  message(STATUS "  Output directory: ${TRANSLATIONS_OUTPUT_DIR}")
  message(STATUS "  Install directory: ${TRANSLATION_INSTALL_DIR}")
endfunction()

# =============================================================================
# 部署Qt应用
# =============================================================================
function(deploy_qt_application TARGET)
  if(NOT TARGET)
    message(FATAL_ERROR "deploy_qt_application: 必须指定TARGET参数")
  endif()

  # 设置部署工具选项
  set(deploy_tool_options_arg "")

  if(WIN32)
    # Windows平台特定的部署选项
    set(deploy_tool_options_arg "--compiler-runtime" "--openssl-root"
                                "${VCPKG_ROOT}/installed/${ARCH_DIR}-windows")
  endif()

  # 生成部署脚本
  qt_generate_deploy_app_script(TARGET ${TARGET} OUTPUT_SCRIPT deploy_script
                                DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg})

  # 安装部署脚本
  install(SCRIPT ${deploy_script})
endfunction()
