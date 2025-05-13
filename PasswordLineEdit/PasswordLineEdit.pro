include(../common.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cc \
    mainwindow.cc \
    passwordlineedit.cc

HEADERS += \
    mainwindow.hpp \
    passwordlineedit.hpp


RESOURCES += \
    resources.qrc

win32: LIBS += -luser32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
