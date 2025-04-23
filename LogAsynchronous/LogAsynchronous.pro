include(../common.pri)

QT       += core gui concurrent widgets

SOURCES += \
        logasync.cpp \
        logfile.cc \
        main.cpp \
        mainwindow.cpp

HEADERS += \
    logasync.h \
    logfile.hpp \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
