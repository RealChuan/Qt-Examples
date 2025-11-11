set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp progressbar.hpp
                    progressbar.cc)

qt_add_executable(progress_bar ${PROJECT_SOURCES})
target_link_libraries(progress_bar PRIVATE Qt::Widgets)
