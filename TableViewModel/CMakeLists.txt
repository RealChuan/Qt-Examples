set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    buttondelegate.h
    buttondelegate.cpp
    comboboxdelegate.h
    comboboxdelegate.cpp
    displaydata.hpp
    displaytablemodel.hpp
    displaytablemodel.cc
    displaytableview.hpp
    displaytableview.cc
    progressbardelegate.h
    progressbardelegate.cpp
    richtextitemdelegate.hpp
    richtextitemdelegate.cc
    sortfilterproxymodel.h
    sortfilterproxymodel.cpp
    stardelegate.cpp
    stardelegate.h
    stareditor.cpp
    stareditor.h
    starrating.cpp
    starrating.h)

qt_add_executable(TableViewModel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(TableViewModel PRIVATE Qt6::Widgets)
qt_finalize_executable(TableViewModel)
