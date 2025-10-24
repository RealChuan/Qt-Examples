# =============================================================================
# Qt Basic Configuration
# =============================================================================

if(CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "C:\\Qt\\6.10.0\\msvc2022_64")
  message(STATUS "Setting Windows Qt path: C:\\Qt\\6.10.0\\msvc2022_64")
elseif(CMAKE_HOST_APPLE)
  # macOS路径 - 可以根据需要填写具体路径 list(APPEND CMAKE_PREFIX_PATH "/path/to/qt/macos")
  message(STATUS "macOS Qt path - configure as needed")
elseif(CMAKE_HOST_LINUX)
  list(APPEND CMAKE_PREFIX_PATH "/opt/Qt/6.10.0/gcc_64")
  message(STATUS "Setting Linux Qt path: /opt/Qt/6.10.0/gcc_64")
endif()

function(setup_qt_definitions)
  add_definitions(-DQT_DEPRECATED_WARNINGS
                  -DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
  message(
    STATUS
      "Qt definitions set: QT_DEPRECATED_WARNINGS, QT_DISABLE_DEPRECATED_BEFORE=0x060000"
  )
endfunction()

setup_qt_definitions()
