set(PROJECT_SOURCES sqlitetest.cc sqlitetest.hpp sqliteutils.cc sqliteutils.hpp
                    sqlitewal_unittest.cc)

qt_add_executable(sqlitewal_unittest ${PROJECT_SOURCES})
target_link_libraries(sqlitewal_unittest PRIVATE Qt::Core Qt::Sql Qt::Test)
