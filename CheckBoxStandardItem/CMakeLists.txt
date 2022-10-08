set(PROJECT_SOURCES
        main.cc
        mainwindow.cc
        mainwindow.hpp
        checkboxstandarditem.hpp
        checkboxstandarditem.cc)

qt_add_executable(CheckBoxStandardItem MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(CheckBoxStandardItem PRIVATE Qt6::Widgets)
qt_finalize_executable(CheckBoxStandardItem)