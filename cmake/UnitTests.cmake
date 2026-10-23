# cmake-format: off
# 单元测试（CTest）
#
#   add_unit_test(<target> SOURCES <file>... LIBRARIES <target>...
#                 [INCLUDES <target>...] [ENV_TARGETS <target>...]
#                 [OUTPUT_DIR <dir>] [TIMEOUT <seconds>])
#   test_environment(<out_var> TARGETS <target>...)
#
# ENV_TARGETS 是 LIBRARIES 之外还要进运行期库搜索路径的目标：ctest 直接拉起
# exe，不经过启动脚本。OUTPUT_DIR 依赖 OutputDirectories.cmake 的
# set_target_output_dir()。
# cmake-format: on

function(test_environment out_var)
  cmake_parse_arguments(ARG "" "" "TARGETS" ${ARGN})

  set(_dirs "")
  foreach(_target IN LISTS ARG_TARGETS)
    if(TARGET ${_target})
      list(APPEND _dirs "$<TARGET_FILE_DIR:${_target}>")
    endif()
  endforeach()

  if(NOT _dirs)
    set(${out_var}
        ""
        PARENT_SCOPE)
    return()
  endif()

  if(WIN32)
    list(JOIN _dirs ";" _joined)
    set(${out_var}
        "PATH=${_joined};$ENV{PATH}"
        PARENT_SCOPE)
  elseif(APPLE)
    list(JOIN _dirs ":" _joined)
    set(${out_var}
        "DYLD_LIBRARY_PATH=${_joined}:$ENV{DYLD_LIBRARY_PATH}"
        PARENT_SCOPE)
  else()
    list(JOIN _dirs ":" _joined)
    set(${out_var}
        "LD_LIBRARY_PATH=${_joined}:$ENV{LD_LIBRARY_PATH}"
        PARENT_SCOPE)
  endif()
endfunction()

function(add_unit_test target_name)
  cmake_parse_arguments(ARG "" "OUTPUT_DIR;TIMEOUT"
                        "SOURCES;LIBRARIES;INCLUDES;ENV_TARGETS" ${ARGN})

  if(NOT ARG_SOURCES)
    message(FATAL_ERROR "add_unit_test(${target_name}): SOURCES 不能为空")
  endif()

  if(NOT ARG_TIMEOUT)
    set(ARG_TIMEOUT 120)
  endif()

  add_executable(${target_name} ${ARG_SOURCES})
  target_link_libraries(${target_name} PRIVATE ${ARG_LIBRARIES} ${ARG_INCLUDES})

  if(ARG_OUTPUT_DIR)
    set_target_output_dir(${target_name} "${ARG_OUTPUT_DIR}")
  endif()

  test_environment(_test_env TARGETS ${ARG_LIBRARIES} ${ARG_ENV_TARGETS})

  if(_test_env)
    add_test(NAME ${target_name} COMMAND ${CMAKE_COMMAND} -E env "${_test_env}"
                                         $<TARGET_FILE:${target_name}>)
  else()
    add_test(NAME ${target_name} COMMAND $<TARGET_FILE:${target_name}>)
  endif()
  set_tests_properties(${target_name} PROPERTIES TIMEOUT ${ARG_TIMEOUT})
endfunction()
