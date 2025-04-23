include(../common.pri)

QT -= gui
QT += network

CONFIG += console
CONFIG -= app_bundle

SOURCES += \
        main.cc \
        receivethread.cc \
        sendthread.cc

HEADERS += \
    receivethread.hpp \
    sendthread.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
