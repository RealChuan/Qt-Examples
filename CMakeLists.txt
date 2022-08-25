cmake_minimum_required(VERSION 3.16)

project(Qt-Examples VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PREFIX_PATH "C:\\Qt\\6.3.0\\msvc2019_64")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#qt_standard_project_setup() 这个识别不了

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Charts)
find_package(Qt6 REQUIRED COMPONENTS Network)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Sql)

add_subdirectory(Battery)
add_subdirectory(BubbleWindow)
add_subdirectory(Chart)
add_subdirectory(Clock)
add_subdirectory(DashBoard)
add_subdirectory(DragDrop)
add_subdirectory(FlowLayout)
add_subdirectory(GridViewModel)
add_subdirectory(HttpClient)
add_subdirectory(LogAsynchronous)
add_subdirectory(MulClient)
add_subdirectory(MulServer)
add_subdirectory(NavigationProgressBar)
add_subdirectory(ProgressArc)
add_subdirectory(ProgressBar)
add_subdirectory(ReactorServer)
add_subdirectory(SlipButton)
add_subdirectory(SqlTabview)
add_subdirectory(TableViewModel)
add_subdirectory(Thread)
add_subdirectory(TreeViewModel)
add_subdirectory(Validator)