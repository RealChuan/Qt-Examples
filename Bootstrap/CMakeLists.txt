set(PROJECT_SOURCES bootstrap.hpp main.cc)

if(CMAKE_HOST_WIN32)
  list(APPEND PROJECT_SOURCES bootstrap_win.cc)
elseif(CMAKE_HOST_APPLE)
  list(APPEND PROJECT_SOURCES bootstrap_mac.cc)
else()
  list(APPEND PROJECT_SOURCES bootstrap_linux.cc)
endif()

qt_add_resources(SOURCES resouce.qrc)

qt_add_executable(Bootstrap MANUAL_FINALIZATION ${PROJECT_SOURCES} ${SOURCES})
target_link_libraries(Bootstrap PRIVATE Qt6::Widgets)
qt_finalize_executable(Bootstrap)
