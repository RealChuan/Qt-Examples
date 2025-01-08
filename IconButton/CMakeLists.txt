set(PROJECT_SOURCES iconfliter.cc iconfliter.hpp main.cc mainwindow.cc
                    mainwindow.hpp)

qt_add_executable(IconButton MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(IconButton PRIVATE Qt::Widgets)
qt_finalize_executable(IconButton)
