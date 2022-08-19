set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        bubblewidget.h
        bubblewidget.cpp)

qt_add_executable(BubbleWindow MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(BubbleWindow PRIVATE Qt6::Widgets)
qt_finalize_executable(BubbleWindow)
