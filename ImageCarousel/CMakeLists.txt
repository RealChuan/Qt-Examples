set(PROJECT_SOURCES
    imagecarousel.cc
    imagecarousel.hpp
    imagecarouselwidget.hpp
    imagecarouselwidget.cc
    main.cc
    mainwindow.cc
    mainwindow.hpp)

qt_add_executable(ImageCarousel MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(ImageCarousel PRIVATE Qt6::Widgets)
qt_finalize_executable(ImageCarousel)
