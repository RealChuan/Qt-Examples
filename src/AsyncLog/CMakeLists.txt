if(BUILD_TESTS)
  set(PROJECT_SOURCES asynclog.cc asynclog.hpp logbuffer.cc logbuffer.hpp
                      logsink.cc logsink.hpp)

  qt_add_unit_test(asynclog_unittest SOURCES asynclog_unittest.cc
                   ${PROJECT_SOURCES} LIBRARIES Qt::Core)
endif()
