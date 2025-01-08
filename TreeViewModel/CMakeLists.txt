set(PROJECT_SOURCES
    main.cc
    mainwindow.cc
    mainwindow.hpp
    checkboxheaderview.hpp
    checkboxheaderview.cc
    fileinfo.hpp
    fileinfo.cc
    fileitem.hpp
    fileitem.cc
    listview.hpp
    listview.cc
    normaltreeview.hpp
    normaltreeview.cc
    treemodel.hpp
    treemodel.cc
    treeview.hpp
    treeview.cc)

qt_add_executable(TreeViewModel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(TreeViewModel PRIVATE Qt::Widgets)
qt_finalize_executable(TreeViewModel)
