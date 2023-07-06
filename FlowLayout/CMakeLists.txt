set(PROJECT_SOURCES
    main.cc
    mainwindow.cc
    mainwindow.hpp
    flowlayout.h
    flowlayout.cpp
    flowwidget.hpp
    flowwidget.cc)

qt_add_executable(FlowLayout MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(FlowLayout PRIVATE Qt6::Widgets)
qt_finalize_executable(FlowLayout)
