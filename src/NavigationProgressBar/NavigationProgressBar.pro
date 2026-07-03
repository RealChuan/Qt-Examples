include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    main.cc \
    mainwindow.cc \
    navigationprogressbar.cc

HEADERS += \
    mainwindow.hpp \
    navigationprogressbar.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
