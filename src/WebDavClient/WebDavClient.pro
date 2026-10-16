include(../../qmake/ProjectSettings.pri)

QT       += core network testlib

SOURCES += \
    ../HttpClient/httpclient.cc  \
    webdavclient.cc  \
    webdavclient_nonet_unittest.cc

HEADERS += \
    ../HttpClient/httpclient.hpp  \
    webdavclient.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
