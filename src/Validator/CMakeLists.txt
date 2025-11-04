set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp validator.hpp
                    validator.cc)

qt_add_executable(Validator ${PROJECT_SOURCES})
target_link_libraries(Validator PRIVATE Qt::Widgets)
