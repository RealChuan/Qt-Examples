set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp windowsintegration.cc
                    windowsintegration.hpp)

qt_add_executable(windows_integration ${PROJECT_SOURCES})
target_link_libraries(windows_integration PRIVATE Qt::Widgets)
