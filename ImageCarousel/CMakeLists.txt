set(PROJECT_SOURCES
    graphicspixmapitem.cc
    graphicspixmapitem.hpp
    imagecarousel.cc
    imagecarousel.hpp
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(ImageCarousel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(ImageCarousel PRIVATE Qt6::Widgets)
qt_finalize_executable(ImageCarousel)
