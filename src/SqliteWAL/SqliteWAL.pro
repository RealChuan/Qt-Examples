include(../../qmake/ProjectSettings.pri)

QT += core sql testlib

CONFIG += cmdline

SOURCES += \
        sqlitetest.cc \
        sqliteutils.cc \
        sqlitewal_unittest.cc 

HEADERS += \
    sqlitetest.hpp \
    sqliteutils.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
