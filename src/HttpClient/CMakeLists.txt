set(PROJECT_SOURCES httpclient_unittest.cc httpclient.cc httpclient.hpp)

qt_add_executable(httpclient_unittest ${PROJECT_SOURCES})
target_link_libraries(httpclient_unittest PRIVATE Qt::Network Qt::Concurrent
                                                  Qt::Test)
target_include_directories(httpclient_unittest PRIVATE ${CMAKE_SOURCE_DIR}/src)
