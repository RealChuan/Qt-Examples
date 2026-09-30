include(../../qmake/ProjectSettings.pri)

QT += core testlib

CONFIG += cmdline

SOURCES += \
        asynclog.cc \
        asynclog_unittest.cc \
        logbuffer.cc \
        logsink.cc

HEADERS += \
        asynclog.hpp \
        logbuffer.hpp \
        logsink.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
