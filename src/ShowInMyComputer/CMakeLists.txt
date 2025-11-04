set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp showinmycomputer.cc
                    showinmycomputer.hpp)

qt_add_executable(ShowInMyComputer ${PROJECT_SOURCES})
target_link_libraries(ShowInMyComputer PRIVATE Qt::Widgets)
