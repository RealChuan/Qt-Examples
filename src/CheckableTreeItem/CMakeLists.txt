set(PROJECT_SOURCES checkabletreeitem.hpp checkabletreeitem.cc main.cc
                    mainwindow.cc mainwindow.hpp)

qt_add_executable(CheckableTreeItem MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(CheckableTreeItem PRIVATE Qt::Widgets)
qt_finalize_executable(CheckableTreeItem)
