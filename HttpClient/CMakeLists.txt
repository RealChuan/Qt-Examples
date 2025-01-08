set(PROJECT_SOURCES main.cpp httpclient.cc httpclient.hpp)

qt_add_executable(HttpClient MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(HttpClient PRIVATE Qt::Network Qt::Concurrent Qt::Test)
qt_finalize_executable(HttpClient)
