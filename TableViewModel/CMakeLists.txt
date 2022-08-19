set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        buttondelegate.h
        buttondelegate.cpp
        comboboxDelegate.h
        comboboxDelegate.cpp
        progressbardelegate.h
        progressbardelegate.cpp
        richtextitemdelegate.hpp
        richtextitemdelegate.cc
        sortfilterproxymodel.h
        sortfilterproxymodel.cpp
        student.h
        studenttablemodel.h
        studenttablemodel.cpp
        stuedenttable.h
        stuedenttable.cpp)

qt_add_executable(TableViewModel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(TableViewModel PRIVATE Qt6::Widgets)
qt_finalize_executable(TableViewModel)