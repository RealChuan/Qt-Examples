qt_add_executable(appNavigationProgressBarQuick main.cc)
target_link_libraries(appNavigationProgressBarQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appNavigationProgressBarQuick
  URI
  NavigationProgressBarQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  NavigationProgressBar.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/NavigationProgressBarQuick")
