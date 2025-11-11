set(PROJECT_SOURCES
    accepterthread.cc
    accepterthread.hpp
    connectioncallbacks.cc
    connectioncallbacks.hpp
    main.cpp
    reactorserver.cc
    reactorserver.hpp
    subreactor.cc
    subreactor.hpp
    tcpconnection.cc
    tcpconnection.hpp
    tcpserver.cc
    tcpserver.hpp
    threadpool.cc
    threadpool.hpp
    workerthread.cc
    workerthread.hpp)

qt_add_executable(reactor_server ${PROJECT_SOURCES})
target_link_libraries(reactor_server PRIVATE Qt::Network)
target_include_directories(reactor_server PRIVATE ${CMAKE_SOURCE_DIR}/src)
