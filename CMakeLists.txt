cmake_minimum_required(VERSION 3.25.1)

include(cmake/macOSDeploymentTarget.cmake)
include(cmake/QtSetup.cmake)

project(
  Qt-Examples
  VERSION 0.0.1
  DESCRIPTION "Examples for Qt"
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-Examples"
  LANGUAGES CXX)

include(cmake/ProjectSettings.cmake)

find_package(
  Qt6 REQUIRED
  COMPONENTS Core
             Widgets
             Network
             Concurrent
             Sql
             Quick
             Test)

qt_standard_project_setup()

include(cmake/QtUtilities.cmake)

add_subdirectory(src)

include(cmake/CMakeDebugInfo.cmake)
