set(PROJECT_SOURCES httpclient.cc httpclient.hpp)

# 需要测试服务器的集成测试
qt_add_executable(httpclient_unittest httpclient_unittest.cc ${PROJECT_SOURCES})
target_link_libraries(httpclient_unittest PRIVATE Qt::Network Qt::Concurrent
                                                  Qt::Test)
target_include_directories(httpclient_unittest PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 无网络测试：不需要测试服务器
qt_add_executable(httpclient_nonet_unittest httpclient_nonet_unittest.cc
                  ${PROJECT_SOURCES})
target_link_libraries(httpclient_nonet_unittest PRIVATE Qt::Network
                                                        Qt::Concurrent Qt::Test)
target_include_directories(httpclient_nonet_unittest
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)
