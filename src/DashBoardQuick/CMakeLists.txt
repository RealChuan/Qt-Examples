qt_add_executable(appDashBoardQuick main.cc)
target_link_libraries(appDashBoardQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appDashBoardQuick
  URI
  DashBoardQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  DashBoard.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DashBoardQuick")
