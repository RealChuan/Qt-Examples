set(PROJECT_SOURCES
    logasync.cpp
    logasync.h
    logfile.cc
    logfile.hpp
    main.cpp
    mainwindow.cpp
    mainwindow.h)

qt_add_executable(LogAsynchronous MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(LogAsynchronous PRIVATE Qt6::Widgets Qt6::Concurrent)
qt_finalize_executable(LogAsynchronous)
