set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        slipbutton.h
        slipbutton.cpp)

qt_add_executable(SlipButton MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(SlipButton PRIVATE Qt6::Widgets)
qt_finalize_executable(SlipButton)
