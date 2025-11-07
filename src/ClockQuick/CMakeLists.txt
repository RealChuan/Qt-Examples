qt_add_executable(appClockQuick main.cc)
target_link_libraries(appClockQuick PRIVATE Qt::Quick)

qt_add_qml_module(
  appClockQuick
  URI
  ClockQuick
  VERSION
  1.0
  QML_FILES
  Main.qml
  Clock.qml
  Hand.qml
  OUTPUT_DIRECTORY
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ClockQuick")
