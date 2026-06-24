set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp batterywidget.hpp
                    batterywidget.cc)

qt_add_executable(Battery ${PROJECT_SOURCES})
target_link_libraries(Battery PRIVATE Qt::Widgets)
