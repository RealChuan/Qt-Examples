qt_add_executable(BatteryQuick main.cc)

qt_add_qml_module(
  BatteryQuick
  URI
  BatteryQuickResources
  VERSION
  1.0
  QML_FILES
  Main.qml
  Battery.qml)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1. If
# you are developing for iOS or macOS you should consider setting an explicit,
# fixed bundle identifier manually though.
set_target_properties(
  BatteryQuick
  PROPERTIES # MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BatteryQuick
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
             MACOSX_BUNDLE TRUE
             WIN32_EXECUTABLE TRUE)

target_link_libraries(BatteryQuick PRIVATE Qt::Quick)

include(GNUInstallDirs)
install(
  TARGETS BatteryQuick
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
