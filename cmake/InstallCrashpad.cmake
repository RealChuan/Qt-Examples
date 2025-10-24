# =============================================================================
# Copy Crashpad Handler to Output Directory
# =============================================================================

macro(get_crashpad_tools_path output_var)
  if(NOT crashpad_DIR)
    message(FATAL_ERROR "crashpad_DIR is not set")
  endif()

  string(REPLACE "share/crashpad" "tools/crashpad" ${output_var}
                 "${crashpad_DIR}")

  if(NOT EXISTS "${${output_var}}")
    message(FATAL_ERROR "Crashpad tools directory not found: ${${output_var}}")
  endif()

  message(STATUS "Crashpad tools directory: ${${output_var}}")
endmacro()

function(setup_crashpad_handler TARGET_DIR)
  get_crashpad_tools_path(crash_handler_path)

  if(NOT EXISTS "${TARGET_DIR}")
    file(MAKE_DIRECTORY "${TARGET_DIR}")
  endif()

  # 检查是否需要复制
  set(need_copy FALSE)

  # 获取源目录中的所有文件
  file(GLOB crashpad_files "${crash_handler_path}/*")

  # 检查每个源文件在目标目录中是否存在且不是最新的
  foreach(source_file ${crashpad_files})
    get_filename_component(filename "${source_file}" NAME)
    set(target_file "${TARGET_DIR}/${filename}")

    if(NOT EXISTS "${target_file}")
      set(need_copy TRUE)
      break()
    else()
      file(TIMESTAMP "${source_file}" source_timestamp)
      file(TIMESTAMP "${target_file}" target_timestamp)

      if(source_timestamp GREATER target_timestamp)
        set(need_copy TRUE)
        break()
      endif()
    endif()
  endforeach()

  if(need_copy)
    message(STATUS "Copying crashpad handler to ${TARGET_DIR}")

    file(
      INSTALL "${crash_handler_path}/"
      DESTINATION "${TARGET_DIR}"
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
      PATTERN
        "crashpad_handler"
        PERMISSIONS
          OWNER_READ
          OWNER_WRITE
          OWNER_EXECUTE
          GROUP_READ
          GROUP_EXECUTE
          WORLD_READ
          WORLD_EXECUTE)
  else()
    message(STATUS "Crashpad handler is up to date in ${TARGET_DIR}")
  endif()
endfunction()
