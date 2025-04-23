include(../common.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    buttondelegate.cpp \
    comboboxdelegate.cpp \
    displaytablemodel.cc \
    displaytableview.cc \
    main.cpp \
    mainwindow.cpp \
    progressbardelegate.cpp \
    richtextitemdelegate.cc \
    sortfilterproxymodel.cpp \
    stardelegate.cpp \
    stareditor.cpp \
    starrating.cpp

HEADERS += \
    buttondelegate.h \
    comboboxdelegate.h \
    displaydata.hpp \
    displaytablemodel.hpp \
    displaytableview.hpp \
    mainwindow.h \
    progressbardelegate.h \
    richtextitemdelegate.hpp \
    sortfilterproxymodel.h \
    stardelegate.h \
    stareditor.h \
    starrating.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
