set(PROJECT_SOURCES main.cc sqlutils.cc sqlutils.hpp)

qt_add_executable(SqliteWAL MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(SqliteWAL PRIVATE Qt6::Core Qt6::Sql)
qt_finalize_executable(SqliteWAL)
