set(PROJECT_SOURCES
    carousel3dview.cc
    carousel3dview.hpp
    interactiveimageitem.cc
    interactiveimageitem.hpp
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(Carousel3DView ${PROJECT_SOURCES})
target_link_libraries(Carousel3DView PRIVATE Qt::Widgets)
