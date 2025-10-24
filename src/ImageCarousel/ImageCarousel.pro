include(../../qmake/ProjectSettings.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    graphicspixmapitem.cc \
    imagecarousel.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    graphicspixmapitem.hpp \
    imagecarousel.hpp \
    mainwindow.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
