set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    draglistwidget.h
    draglistwidget.cpp
    droplistwidget.h
    droplistwidget.cpp)

qt_add_executable(DragDrop MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(DragDrop PRIVATE Qt::Widgets)
qt_finalize_executable(DragDrop)
