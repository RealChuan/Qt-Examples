set(PROJECT_SOURCES main.cpp receivethread.cpp receivethread.h sendthread.h
                    sendthread.cpp)

qt_add_executable(SimpleUdp ${PROJECT_SOURCES})
target_link_libraries(SimpleUdp PRIVATE Qt::Network)
