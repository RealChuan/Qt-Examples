set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        sql.h
        sql.cpp)

qt_add_executable(SqlTabview MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(SqlTabview PRIVATE Qt6::Widgets Qt6::Sql)
qt_finalize_executable(SqlTabview)
