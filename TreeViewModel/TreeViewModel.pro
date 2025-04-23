include(../common.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    checkboxheaderview.cc \
    fileinfo.cc \
    fileitem.cc \
    listview.cc \
    main.cc \
    mainwindow.cc \
    normaltreeview.cc \
    treemodel.cc \
    treeview.cc

HEADERS += \
    checkboxheaderview.hpp \
    fileinfo.hpp \
    fileitem.hpp \
    listview.hpp \
    mainwindow.hpp \
    normaltreeview.hpp \
    treemodel.hpp \
    treeview.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
