# cmake-format: off
# Qt 测试入口：在 cmake/UnitTests.cmake 之上补 Qt::Test、Qt6::Core 与 AUTOMOC。
# 参数与 add_unit_test() 相同。
# cmake-format: on

include("${CMAKE_CURRENT_LIST_DIR}/UnitTests.cmake")

function(qt_add_unit_test target_name)
  cmake_parse_arguments(ARG "" "OUTPUT_DIR;TIMEOUT"
                        "SOURCES;LIBRARIES;INCLUDES;ENV_TARGETS" ${ARGN})

  add_unit_test(
    ${target_name}
    SOURCES
    ${ARG_SOURCES}
    LIBRARIES
    ${ARG_LIBRARIES}
    Qt::Test
    ENV_TARGETS
    ${ARG_ENV_TARGETS}
    Qt6::Core
    INCLUDES
    ${ARG_INCLUDES}
    OUTPUT_DIR
    "${ARG_OUTPUT_DIR}"
    TIMEOUT
    "${ARG_TIMEOUT}")

  # 不依赖全局 AUTOMOC
  set_target_properties(${target_name} PROPERTIES AUTOMOC ON)
endfunction()
