set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    areachart.h
    areachart.cpp
    callout.h
    callout.cpp
    chartview.h
    chartview.cpp
    datetimechart.h
    datetimechart.cpp
    dialogchart.h
    dialogchart.cpp
    dynamicchart.h
    dynamicchart.cpp
    dynamicchartx.h
    dynamicchartx.cpp
    linechart.h
    linechart.cpp
    normalchartdata.h
    normalchartdata.cpp
    piechart.h
    piechart.cpp
    stackedbarchart.h
    stackedbarchart.cpp)

qt_add_executable(Chart MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(Chart PRIVATE Qt6::Widgets Qt6::Charts)
qt_finalize_executable(Chart)
