set(PROJECT_SOURCES sqlitetest.cc sqlitetest.hpp sqliteutils.cc sqliteutils.hpp
                    main.cc)

qt_add_executable(SqliteWAL MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(SqliteWAL PRIVATE Qt::Core Qt::Sql)
qt_finalize_executable(SqliteWAL)
