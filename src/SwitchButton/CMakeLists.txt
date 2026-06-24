set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp switchbutton.cc
                    switchbutton.hpp)

qt_add_executable(SwitchButton ${PROJECT_SOURCES})
target_link_libraries(SwitchButton PRIVATE Qt::Widgets)
