set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        fileutil.h
        fileutil.cpp
        logasync.h
        logasync.cpp)

qt_add_executable(LogAsynchronous MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(LogAsynchronous PRIVATE Qt6::Widgets Qt6::Concurrent)
qt_finalize_executable(LogAsynchronous)
