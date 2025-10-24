include(../../qmake/ProjectSettings.pri)

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    accepter.cpp \
    main.cpp \
    mainwindow.cpp \
    subreactor.cpp \
    tcpclient.cpp \
    tcpserver.cpp \
    thread.cpp

HEADERS += \
    accepter.h \
    mainwindow.h \
    subreactor.h \
    tcpclient.h \
    tcpserver.h \
    thread.h

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
