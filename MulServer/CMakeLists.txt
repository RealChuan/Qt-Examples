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

qt_add_executable(MulServer MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(MulServer PRIVATE Qt6::Widgets Qt6::Network)
qt_finalize_executable(MulServer)