set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp progressbar.hpp
                    progressbar.cc)

qt_add_executable(ProgressBar ${PROJECT_SOURCES})
target_link_libraries(ProgressBar PRIVATE Qt::Widgets)
