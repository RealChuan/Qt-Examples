set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h switchbutton.cpp
                    switchbutton.h)

qt_add_executable(SwitchButton ${PROJECT_SOURCES})
target_link_libraries(SwitchButton PRIVATE Qt::Widgets)
