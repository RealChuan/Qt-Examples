set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h batterywidget.h
                    batterywidget.cpp)

qt_add_executable(Battery ${PROJECT_SOURCES})
target_link_libraries(Battery PRIVATE Qt::Widgets)
