qt_add_executable(appCircularProgressQuick main.cc)
target_link_libraries(appCircularProgressQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appCircularProgressQuick
  URI
  CircularProgressQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  CircularProgress.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CircularProgressQuick")
