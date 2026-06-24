set(PROJECT_SOURCES circularprogress.cc circularprogress.hpp main.cc
                    mainwindow.cc mainwindow.hpp)

qt_add_executable(circular_progress ${PROJECT_SOURCES})
target_link_libraries(circular_progress PRIVATE Qt::Widgets)
