set(PROJECT_SOURCES
    bubblewidget.h
    bubblewidget.cpp
    bubblewrapperwidget.cc
    bubblewrapperwidget.hpp
    main.cpp
    mainwindow.cpp
    mainwindow.h)

qt_add_executable(Bubble MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Bubble PRIVATE Qt::Widgets)
qt_finalize_executable(Bubble)
