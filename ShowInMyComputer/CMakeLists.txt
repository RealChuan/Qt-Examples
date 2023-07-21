set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp showinmycomputer.cc
                    showinmycomputer.hpp)

qt_add_executable(ShowInMyComputer MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(ShowInMyComputer PRIVATE Qt6::Widgets)
qt_finalize_executable(ShowInMyComputer)
