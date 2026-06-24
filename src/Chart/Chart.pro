include(../../qmake/ProjectSettings.pri)

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    areachart.cc \
    callout.cc \
    chartview.cc \
    datetimechart.cc \
    dialogchart.cc \
    dynamicchart.cc \
    dynamicchartx.cc \
    linechart.cc \
    main.cc \
    mainwindow.cc \
    normalchartdata.cc \
    piechart.cc \
    stackedbarchart.cc

HEADERS += \
    areachart.hpp \
    callout.hpp \
    chartview.hpp \
    datetimechart.hpp \
    dialogchart.hpp \
    dynamicchart.hpp \
    dynamicchartx.hpp \
    linechart.hpp \
    mainwindow.hpp \
    normalchartdata.hpp \
    piechart.hpp \
    stackedbarchart.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
