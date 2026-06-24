set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp dashboardwidget.hpp
                    dashboardwidget.cc)

qt_add_executable(DashBoard ${PROJECT_SOURCES})
target_link_libraries(DashBoard PRIVATE Qt::Widgets)
