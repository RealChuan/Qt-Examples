set(PROJECT_SOURCES main.cpp mainwindow.cpp mainwindow.h httpclient.h
                    httpclient.cpp)

qt_add_executable(HttpClient MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(HttpClient PRIVATE Qt6::Widgets Qt6::Network
                                         Qt6::Concurrent)
qt_finalize_executable(HttpClient)
