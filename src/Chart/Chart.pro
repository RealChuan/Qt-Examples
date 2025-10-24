include(../../qmake/ProjectSettings.pri)

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    areachart.cpp \
    callout.cpp \
    chartview.cpp \
    datetimechart.cpp \
    dialogchart.cpp \
    dynamicchart.cpp \
    dynamicchartx.cpp \
    linechart.cpp \
    main.cpp \
    mainwindow.cpp \
    normalchartdata.cpp \
    piechart.cpp \
    stackedbarchart.cpp

HEADERS += \
    areachart.h \
    callout.h \
    chartview.h \
    datetimechart.h \
    dialogchart.h \
    dynamicchart.h \
    dynamicchartx.h \
    linechart.h \
    mainwindow.h \
    normalchartdata.h \
    piechart.h \
    stackedbarchart.h

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
