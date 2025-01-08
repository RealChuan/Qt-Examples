set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h progressarc.h
                    progressarc.cpp)

qt_add_executable(ProgressArc MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(ProgressArc PRIVATE Qt::Widgets)
qt_finalize_executable(ProgressArc)
