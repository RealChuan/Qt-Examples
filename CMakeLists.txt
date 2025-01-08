cmake_minimum_required(VERSION 3.25.1)

include(cmake/qt.cmake)

project(
  Qt-Examples
  VERSION 0.0.1
  DESCRIPTION "Examples for Qt"
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-Examples"
  LANGUAGES CXX)

include(cmake/common.cmake)

find_package(
  Qt6 6.5 REQUIRED
  COMPONENTS Core
             Widgets
             Charts
             Network
             Concurrent
             Sql
             Quick
             Test)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0001 NEW)
# qt_standard_project_setup will set CMAKE_RUNTIME_OUTPUT_DIRECTORY, we need to
# set it back, and use EXECUTABLE_OUTPUT_PATH
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

add_subdirectory(Battery)
add_subdirectory(BatteryQuick)
add_subdirectory(Bootstrap)
add_subdirectory(BubbleWindow)
add_subdirectory(Chart)
add_subdirectory(CheckBoxStandardItem)
add_subdirectory(Clock)
add_subdirectory(DashBoard)
add_subdirectory(DragDrop)
add_subdirectory(FlowLayout)
add_subdirectory(GridViewModel)
add_subdirectory(HttpClient)
add_subdirectory(IconButton)
add_subdirectory(ImageCarousel)
add_subdirectory(LoadingIndicator)
add_subdirectory(LogAsynchronous)
add_subdirectory(MulClient)
add_subdirectory(MulServer)
add_subdirectory(NavigationProgressBar)
add_subdirectory(PasswordLineEdit)
add_subdirectory(ProgressArc)
add_subdirectory(ProgressBar)
add_subdirectory(ReactorServer)
add_subdirectory(SlipButton)
add_subdirectory(SqliteWAL)
add_subdirectory(TableViewModel)
add_subdirectory(Thread)
add_subdirectory(TreeViewModel)
add_subdirectory(Validator)

if(CMAKE_HOST_WIN32)
  add_subdirectory(ShowInMyComputer)
endif()

include(cmake/build_info.cmake)
