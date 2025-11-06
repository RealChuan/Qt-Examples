qt_add_executable(appPasswordInputQuick main.cc)
target_link_libraries(appPasswordInputQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appPasswordInputQuick
  URI
  PasswordInputQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  PasswordInput.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PasswordInputQuick")
