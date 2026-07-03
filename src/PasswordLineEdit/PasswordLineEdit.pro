include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    main.cc \
    mainwindow.cc \
    passwordlineedit.cc

HEADERS += \
    mainwindow.hpp \
    passwordlineedit.hpp

win32: LIBS += -luser32

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
