set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        gridmodel.h
        gridmodel.cpp
        gridview.h
        gridview.cpp)

qt_add_executable(GridViewModel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(GridViewModel PRIVATE Qt6::Widgets)
qt_finalize_executable(GridViewModel)
