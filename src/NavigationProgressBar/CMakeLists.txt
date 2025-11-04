set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h
                    navigationprogressbar.h navigationprogressbar.cpp)

qt_add_executable(NavigationProgressBar ${PROJECT_SOURCES})
target_link_libraries(NavigationProgressBar PRIVATE Qt::Widgets)
