cmake_minimum_required(VERSION 3.16)

project(
  Qt-Examples
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_HOST_WIN32)
  list(APPEND CMAKE_PREFIX_PATH "C:\\Qt\\6.5.0\\msvc2019_64")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# qt_standard_project_setup() 这个识别不了

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network Concurrent Sql)

add_subdirectory(Battery)
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
add_subdirectory(ImageCarousel)
add_subdirectory(LogAsynchronous)
add_subdirectory(MulClient)
add_subdirectory(MulServer)
add_subdirectory(NavigationProgressBar)
add_subdirectory(PasswordLineEdit)
add_subdirectory(ProgressArc)
add_subdirectory(ProgressBar)
add_subdirectory(ReactorServer)
add_subdirectory(SlipButton)
add_subdirectory(SqlTabview)
add_subdirectory(TableViewModel)
add_subdirectory(Thread)
add_subdirectory(TreeViewModel)
add_subdirectory(Validator)
