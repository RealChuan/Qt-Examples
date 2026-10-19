include(../../qmake/ProjectSettings.pri)

QT       += core network testlib

SOURCES += \
    httpclient.cc  \
    httpclient_nonet_unittest.cc

HEADERS += \
    httpclient.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
