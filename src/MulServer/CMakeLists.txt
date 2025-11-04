set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    accepter.h
    accepter.cpp
    tcpclient.h
    tcpclient.cpp
    tcpserver.h
    tcpserver.cpp
    thread.h
    thread.cpp)

qt_add_executable(MulServer ${PROJECT_SOURCES})
target_link_libraries(MulServer PRIVATE Qt::Widgets Qt::Network)
