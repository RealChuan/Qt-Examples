set(PROJECT_SOURCES
    graphicspixmapitem.cc
    graphicspixmapitem.hpp
    imagecarousel.cc
    imagecarousel.hpp
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(ImageCarousel ${PROJECT_SOURCES})
target_link_libraries(ImageCarousel PRIVATE Qt::Widgets)
