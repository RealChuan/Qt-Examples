# =============================================================================
# InstallConfig.cmake - 跨平台安装路径配置
# =============================================================================

# 定义项目名称的小写版本（用于Unix路径）
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWERCASE)

# 设置默认安装前缀（如果未指定）
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(APPLE)
    set(CMAKE_INSTALL_PREFIX
        "/Applications"
        CACHE PATH "Install path prefix" FORCE)
  elseif(UNIX)
    # Linux 安装到 /opt/${PROJECT_NAME}
    set(CMAKE_INSTALL_PREFIX
        "/opt/${PROJECT_NAME}"
        CACHE PATH "Install path prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX
        "C:/Program Files/${PROJECT_NAME}"
        CACHE PATH "Install path prefix" FORCE)
  endif()
endif()

# 平台特定的安装路径配置
if(APPLE)
  # macOS Bundle结构
  set(MAIN_APP_INSTALL_DIR ".") # 安装为.app bundle
  set(TOOL_INSTALL_DIR "${PROJECT_NAME}.app/Contents/MacOS")
  set(LIB_INSTALL_DIR "${PROJECT_NAME}.app/Contents/Frameworks")
  set(PLUGIN_INSTALL_DIR
      "${PROJECT_NAME}.app/Contents/PlugIns/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "${PROJECT_NAME}.app/Contents/Resources")
  set(TRANSLATION_INSTALL_DIR
      "${PROJECT_NAME}.app/Contents/Resources/translations")
elseif(WIN32)
  # Windows安装路径
  set(CMAKE_INSTALL_BINDIR ".") # 可执行文件和DLL在根目录
  set(CMAKE_INSTALL_LIBDIR ".")
  set(MAIN_APP_INSTALL_DIR ".")
  set(TOOL_INSTALL_DIR ".")
  set(LIB_INSTALL_DIR ".")
  set(PLUGIN_INSTALL_DIR "plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "resources")
  set(TRANSLATION_INSTALL_DIR "translations")
else()
  # Linux安装路径 - 基于 /opt/${PROJECT_NAME}
  set(CMAKE_INSTALL_BINDIR ".") # 可执行文件和DLL在根目录
  set(CMAKE_INSTALL_LIBDIR "lib")
  set(MAIN_APP_INSTALL_DIR ".")
  set(TOOL_INSTALL_DIR ".")
  set(LIB_INSTALL_DIR "lib")
  set(PLUGIN_INSTALL_DIR "plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "resources")
  set(TRANSLATION_INSTALL_DIR "translations")
endif()

# 设置标志表示安装配置已加载
set(INSTALL_CONFIG_LOADED TRUE)
