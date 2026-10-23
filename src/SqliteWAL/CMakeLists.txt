if(BUILD_TESTS)
  set(PROJECT_SOURCES sqlitetest.cc sqlitetest.hpp sqliteutils.cc
                      sqliteutils.hpp sqlitewal_unittest.cc)

  qt_add_unit_test(sqlitewal_unittest SOURCES ${PROJECT_SOURCES} LIBRARIES
                   Qt::Core Qt::Sql)
endif()
