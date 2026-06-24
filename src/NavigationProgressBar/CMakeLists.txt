set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp
                    navigationprogressbar.hpp navigationprogressbar.cc)

qt_add_executable(navigation_progress ${PROJECT_SOURCES})
target_link_libraries(navigation_progress PRIVATE Qt::Widgets)
