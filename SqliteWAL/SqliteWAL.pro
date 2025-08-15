include(../common.pri)

QT += core sql

CONFIG += cmdline

SOURCES += \
        main.cc \
        sqlitetest.cc \
        sqliteutils.cc

HEADERS += \
    sqlitetest.hpp \
    sqliteutils.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
