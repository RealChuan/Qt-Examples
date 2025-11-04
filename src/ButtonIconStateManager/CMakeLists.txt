set(PROJECT_SOURCES buttoniconstatemanager.cc buttoniconstatemanager.hpp
                    main.cc mainwindow.cc mainwindow.hpp)

qt_add_executable(ButtonIconStateManager ${PROJECT_SOURCES})
target_link_libraries(ButtonIconStateManager PRIVATE Qt::Widgets)
