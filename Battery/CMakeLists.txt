set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        batterywidget.h
        batterywidget.cpp)

qt_add_executable(Battery MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Battery PRIVATE Qt6::Widgets)
qt_finalize_executable(Battery)
