set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    tcpclient.h
    tcpclient.cpp
    tcpclientthread.h
    tcpclientthread.cpp)

qt_add_executable(MulClient MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(MulClient PRIVATE Qt::Widgets Qt::Network)
qt_finalize_executable(MulClient)
