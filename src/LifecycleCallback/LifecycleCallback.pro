include(../../qmake/ProjectSettings.pri)

QT       += core  testlib

SOURCES += \
    lifecyclecallback_unittest.cc

HEADERS += 

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
