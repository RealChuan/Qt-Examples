set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        test.h
        test.cpp
        thread1.h
        thread1.cpp
        thread2.h
        thread2.cpp
        thread3.h
        thread3.cpp)

qt_add_executable(Thread MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Thread PRIVATE Qt6::Widgets Qt6::Concurrent)
qt_finalize_executable(Thread)
