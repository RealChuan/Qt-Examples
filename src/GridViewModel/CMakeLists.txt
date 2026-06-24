set(PROJECT_SOURCES
    gridmodel.hpp
    gridmodel.cc
    gridview.hpp
    gridview.cc
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(GridViewModel ${PROJECT_SOURCES})
target_link_libraries(GridViewModel PRIVATE Qt::Widgets)
