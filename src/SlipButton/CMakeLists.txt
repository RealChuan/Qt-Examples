set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h slipbutton.h
                    slipbutton.cpp)

qt_add_executable(SlipButton ${PROJECT_SOURCES})
target_link_libraries(SlipButton PRIVATE Qt::Widgets)
