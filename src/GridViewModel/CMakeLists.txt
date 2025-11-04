set(PROJECT_SOURCES
    gridmodel.h
    gridmodel.cpp
    gridview.h
    gridview.cpp
    main.cpp
    mainwindow.cpp
    mainwindow.h)

qt_add_executable(GridViewModel ${PROJECT_SOURCES})
target_link_libraries(GridViewModel PRIVATE Qt::Widgets)
