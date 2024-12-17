set(PROJECT_SOURCES loadingindicator.cc loadingindicator.hpp main.cc
                    mainwindow.cc mainwindow.hpp)

qt_add_resources(SOURCES resource.qrc)

qt_add_executable(LoadingIndicator MANUAL_FINALIZATION ${PROJECT_SOURCES}
                  ${SOURCES})
target_link_libraries(LoadingIndicator PRIVATE Qt6::Widgets)
qt_finalize_executable(LoadingIndicator)
