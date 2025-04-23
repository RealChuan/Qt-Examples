include(../common.pri)

QT += core sql

CONFIG += cmdline

SOURCES += \
        databasetest.cc \
        databaseutils.cc \
        main.cc

HEADERS += \
    databasetest.hpp \
    databaseutils.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
