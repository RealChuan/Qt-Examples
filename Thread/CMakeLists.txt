set(PROJECT_SOURCES
        functions.cc
        functions.hpp
        main.cc
        task.cc
        task.hpp
        thread.cc
        thread.hpp)

qt_add_executable(Thread MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Thread PRIVATE Qt6::Core Qt6::Concurrent) 
qt_finalize_executable(Thread)