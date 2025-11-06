qt_add_executable(appLoadingIndicatorQuick main.cc)
target_link_libraries(appLoadingIndicatorQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appLoadingIndicatorQuick
  URI
  LoadingIndicatorQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  LoadingIndicator.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LoadingIndicatorQuick"
  RESOURCES
  ../LoadingIndicator/resources/loading.gif)
