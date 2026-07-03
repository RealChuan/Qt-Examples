include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    carousel3dview.cc \
    interactiveimageitem.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    carousel3dview.hpp \
    interactiveimageitem.hpp \
    mainwindow.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
