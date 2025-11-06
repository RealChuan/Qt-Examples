set(PROJECT_SOURCES loadingindicator.cc loadingindicator.hpp main.cc
                    mainwindow.cc mainwindow.hpp)

qt_add_resources(SOURCES resources.qrc)

qt_add_executable(loading_indicator ${PROJECT_SOURCES} ${SOURCES})
target_link_libraries(loading_indicator PRIVATE Qt::Widgets)
