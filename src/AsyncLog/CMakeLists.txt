set(PROJECT_SOURCES asynclog.cc asynclog.hpp logbuffer.cc logbuffer.hpp
                    logsink.cc logsink.hpp)

qt_add_executable(asynclog_unittest asynclog_unittest.cc ${PROJECT_SOURCES})
target_link_libraries(asynclog_unittest PRIVATE Qt::Core Qt::Test)
