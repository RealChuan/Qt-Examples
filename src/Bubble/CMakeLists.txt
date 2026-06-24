set(PROJECT_SOURCES
    bubblewidget.hpp
    bubblewidget.cc
    bubblewrapperwidget.cc
    bubblewrapperwidget.hpp
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(Bubble ${PROJECT_SOURCES})
target_link_libraries(Bubble PRIVATE Qt::Widgets)
