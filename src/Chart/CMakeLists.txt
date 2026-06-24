set(PROJECT_SOURCES
    main.cc
    mainwindow.cc
    mainwindow.h
    areachart.h
    areachart.cc
    callout.h
    callout.cc
    chartview.h
    chartview.cc
    datetimechart.h
    datetimechart.cc
    dialogchart.h
    dialogchart.cc
    dynamicchart.h
    dynamicchart.cc
    dynamicchartx.h
    dynamicchartx.cc
    linechart.h
    linechart.cc
    normalchartdata.h
    normalchartdata.cc
    piechart.h
    piechart.cc
    stackedbarchart.h
    stackedbarchart.cc)

qt_add_executable(Chart ${PROJECT_SOURCES})
target_link_libraries(Chart PRIVATE Qt::Widgets Qt::Charts)
