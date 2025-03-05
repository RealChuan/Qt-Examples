set(PROJECT_SOURCES httpclient_unittest.cc httpclient.cc httpclient.hpp)

qt_add_executable(httpclient_unittest MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(httpclient_unittest PRIVATE Qt::Network Qt::Concurrent
                                                  Qt::Test)
qt_finalize_executable(httpclient_unittest)
