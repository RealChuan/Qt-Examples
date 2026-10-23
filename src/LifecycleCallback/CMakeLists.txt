if(BUILD_TESTS)
  set(PROJECT_SOURCES lifecyclecallback.hpp lifecyclecallback_qt.hpp
                      lifecyclecallback_unittest.cc)

  qt_add_unit_test(lifecyclecallback_unittest SOURCES ${PROJECT_SOURCES})
endif()
