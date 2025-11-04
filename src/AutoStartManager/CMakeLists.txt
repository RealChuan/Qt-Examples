set(PROJECT_SOURCES autostartmanager.hpp main.cc mainwindow.cpp mainwindow.h)

if(CMAKE_HOST_WIN32)
  list(APPEND PROJECT_SOURCES autostartmanager_windows.cc)
elseif(CMAKE_HOST_APPLE)
  list(APPEND PROJECT_SOURCES autostartmanager_macos.cc)
else()
  list(APPEND PROJECT_SOURCES autostartmanager_linux.cc)
endif()

qt_add_executable(AutoStartManager ${PROJECT_SOURCES})
target_link_libraries(AutoStartManager PRIVATE Qt::Widgets)
