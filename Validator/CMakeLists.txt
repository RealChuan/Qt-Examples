set(PROJECT_SOURCES
        main.cc
        mainwindow.cc
        mainwindow.hpp
        validator.hpp
        validator.cc)

qt_add_executable(Validator MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Validator PRIVATE Qt6::Widgets)
qt_finalize_executable(Validator)