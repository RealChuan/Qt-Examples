include(../../qmake/ProjectSettings.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cc \
    mainwindow.cc

HEADERS += \
    autostartmanager.hpp \
    mainwindow.hpp

win32 {
    SOURCES += autostartmanager_windows.cc
}

macx {
    SOURCES += autostartmanager_macos.cc
}

unix:!macx {
    SOURCES += autostartmanager_linux.cc
}

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
