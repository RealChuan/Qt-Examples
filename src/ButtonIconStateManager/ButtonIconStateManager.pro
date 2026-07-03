include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    buttoniconstatemanager.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    buttoniconstatemanager.hpp \
    mainwindow.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
