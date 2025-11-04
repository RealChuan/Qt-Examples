set(PROJECT_SOURCES checkabletreeitem.hpp checkabletreeitem.cc main.cc
                    mainwindow.cc mainwindow.hpp)

qt_add_executable(CheckableTreeItem ${PROJECT_SOURCES})
target_link_libraries(CheckableTreeItem PRIVATE Qt::Widgets)
