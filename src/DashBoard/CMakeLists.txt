set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h dashboardwidget.h
                    dashboardwidget.cpp)

qt_add_executable(DashBoard ${PROJECT_SOURCES})
target_link_libraries(DashBoard PRIVATE Qt::Widgets)
