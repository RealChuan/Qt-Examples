if(BUILD_TESTS)
  set(PROJECT_SOURCES httpclient.cc httpclient.hpp)

  # 需要测试服务器的集成测试：手动起 unittests/test_server.py 后运行，不加入 CTest
  qt_add_executable(httpclient_unittest httpclient_unittest.cc
                    ${PROJECT_SOURCES})
  target_link_libraries(httpclient_unittest PRIVATE Qt::Network Qt::Test)
  target_include_directories(httpclient_unittest
                             PRIVATE ${CMAKE_SOURCE_DIR}/src)

  # 无网络测试：不需要测试服务器，加入 CTest
  qt_add_unit_test(
    httpclient_nonet_unittest SOURCES httpclient_nonet_unittest.cc
    ${PROJECT_SOURCES} LIBRARIES Qt::Network)
  target_include_directories(httpclient_nonet_unittest
                             PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()
