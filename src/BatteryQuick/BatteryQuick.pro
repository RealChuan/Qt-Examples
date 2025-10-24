include(../../qmake/ProjectSettings.pri)

QT += quick

SOURCES += \
        main.cc

DESTDIR = $$RUNTIME_OUTPUT_DIRECTORY

resources.files = Main.qml 
resources.prefix = /qt/qml/BatteryQuickResources
RESOURCES += resources

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

resources.files += \
    Battery.qml
