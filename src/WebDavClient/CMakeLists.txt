set(PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/HttpClient/httpclient.cc
    ${CMAKE_SOURCE_DIR}/src/HttpClient/httpclient.hpp webdavclient.cc
    webdavclient.hpp)

# 需要测试服务器的集成测试：unittests/dav_server.py（127.0.0.1:8001）， 服务器不可达时整批自动跳过
qt_add_executable(webdavclient_unittest webdavclient_unittest.cc
                  ${PROJECT_SOURCES})
target_link_libraries(webdavclient_unittest PRIVATE Qt::Network Qt::Test)
target_include_directories(webdavclient_unittest
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 无网络测试：207 解析 / 错误分类 / 锁令牌表，不需要测试服务器
qt_add_executable(webdavclient_nonet_unittest webdavclient_nonet_unittest.cc
                  ${PROJECT_SOURCES})
target_link_libraries(webdavclient_nonet_unittest PRIVATE Qt::Network Qt::Test)
target_include_directories(webdavclient_nonet_unittest
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)

# WsgiDAV 真实服务器符合性测试：unittests/wsgidav_server.py（127.0.0.1:8002， pip install
# wsgidav 后启动），服务器不可达时整批自动跳过
qt_add_executable(webdavclient_conformance_unittest
                  webdavclient_conformance_unittest.cc ${PROJECT_SOURCES})
target_link_libraries(webdavclient_conformance_unittest PRIVATE Qt::Network
                                                                Qt::Test)
target_include_directories(webdavclient_conformance_unittest
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)
