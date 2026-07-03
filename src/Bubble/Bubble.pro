include(../../qmake/ProjectSettings.pri)

QT       += core gui widgets

SOURCES += \
    bubblewidget.cc \
    bubblewrapperwidget.cc \
    main.cc \
    mainwindow.cc

HEADERS += \
    bubblewidget.hpp \
    bubblewrapperwidget.hpp \
    mainwindow.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
