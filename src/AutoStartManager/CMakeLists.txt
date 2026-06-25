set(PROJECT_SOURCES autostartmanager.hpp main.cc mainwindow.cc mainwindow.hpp)

# 按目标平台（而非宿主机平台）选择实现，支持交叉编译
if(WIN32)
  list(APPEND PROJECT_SOURCES autostartmanager_windows.cc)
elseif(APPLE)
  list(APPEND PROJECT_SOURCES autostartmanager_macos.cc)
else()
  list(APPEND PROJECT_SOURCES autostartmanager_linux.cc)
endif()

qt_add_executable(auto_start_manager ${PROJECT_SOURCES})
target_link_libraries(auto_start_manager PRIVATE Qt::Widgets)
