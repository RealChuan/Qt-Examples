set(PROJECT_SOURCES
    functions.cc
    functions.hpp
    main.cc
    task.cc
    task.hpp
    thread.cc
    thread.hpp)

qt_add_executable(Thread ${PROJECT_SOURCES})
target_link_libraries(Thread PRIVATE Qt::Core Qt::Concurrent)
