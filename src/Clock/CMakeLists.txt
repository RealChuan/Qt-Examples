set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp clockwidget.hpp
                    clockwidget.cc)

qt_add_executable(Clock ${PROJECT_SOURCES})
target_link_libraries(Clock PRIVATE Qt::Widgets)
