include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    loadingindicator.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    loadingindicator.hpp \
    mainwindow.hpp

RESOURCES += \
    resources.qrc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
