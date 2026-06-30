set(PROJECT_SOURCES lifecyclecallback.hpp lifecyclecallback_qt.hpp
                    lifecyclecallback_unittest.cc)

qt_add_executable(lifecyclecallback_unittest ${PROJECT_SOURCES})
target_link_libraries(lifecyclecallback_unittest PRIVATE Qt::Test)
