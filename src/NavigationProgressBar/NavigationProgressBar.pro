include(../../qmake/ProjectSettings.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    navigationprogressbar.cpp

HEADERS += \
    mainwindow.h \
    navigationprogressbar.h

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
