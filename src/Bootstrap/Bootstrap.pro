include(../../qmake/ProjectSettings.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cc

HEADERS += \
    bootstrap.hpp

win32 {
    SOURCES += bootstrap_win.cc
}

macx {
    SOURCES += bootstrap_mac.cc
}

unix:!macx {
    SOURCES += bootstrap_linux.cc
}

RESOURCES += \
    resouce.qrc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
