include(../../qmake/ProjectSettings.pri)

QT       += core gui network testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    httpclient.cc  \
    httpclient_unittest.cc

HEADERS += \
    httpclient.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
