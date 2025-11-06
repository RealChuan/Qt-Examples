qt_add_executable(appLoadingOverlayQuick main.cc)
target_link_libraries(appLoadingOverlayQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appLoadingOverlayQuick
  URI
  LoadingOverlayQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  LoadingOverlay.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LoadingOverlayQuick")
