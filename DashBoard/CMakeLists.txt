set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        dashboardwidget.h
        dashboardwidget.cpp)

qt_add_executable(DashBoard MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(DashBoard PRIVATE Qt6::Widgets)
qt_finalize_executable(DashBoard)
