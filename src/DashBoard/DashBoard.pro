include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    dashboardwidget.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    dashboardwidget.hpp \
    mainwindow.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
