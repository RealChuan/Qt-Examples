set(PROJECT_SOURCES main.cc mainwindow.cc mainwindow.hpp passwordlineedit.cc
                    passwordlineedit.hpp)

qt_add_resources(PROJECT_SOURCES resources.qrc)

qt_add_executable(PasswordLineEdit MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(PasswordLineEdit PRIVATE Qt::Widgets)
qt_finalize_executable(PasswordLineEdit)
