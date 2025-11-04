qt_add_executable(appBatteryQuick main.cc)
target_link_libraries(appBatteryQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appBatteryQuick
  URI
  BatteryQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  Battery.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BatteryQuick")
