set(PROJECT_SOURCES
        main.cpp
        receivethread.cpp
        receivethread.h
        sendthread.h
        sendthread.cpp)

qt_add_executable(SimpleUdp MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(SimpleUdp PRIVATE Qt6::Network)
qt_finalize_executable(SimpleUdp)
