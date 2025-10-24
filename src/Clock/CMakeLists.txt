set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h clockwidget.h
                    clockwidget.cpp)

qt_add_executable(Clock MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Clock PRIVATE Qt::Widgets)
qt_finalize_executable(Clock)
