include(../../qmake/ProjectSettings.pri)

QT       += core network

SOURCES += \
    accepterthread.cc \
    connectioncallbacks.cc \
    main.cpp \
    reactorserver.cc \
    subreactor.cc \
    tcpconnection.cc \
    tcpserver.cc \
    threadpool.cc \
    workerthread.cc

HEADERS += \
    accepterthread.hpp \
    connectioncallbacks.hpp \
    reactorserver.hpp \
    subreactor.hpp \
    tcpconnection.hpp \
    tcpserver.hpp \
    threadpool.hpp \
    workerthread.hpp

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
