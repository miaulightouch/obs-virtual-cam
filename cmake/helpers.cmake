include_guard(GLOBAL)

# Add cmake module directories from obs-studio
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/buildspec.json" buildspec)
string(
  JSON
  obs_version
  GET
  ${buildspec}
  "dependencies"
  "obs-studio"
  "version")
find_path(DEPS_ROOT "obs-studio-${obs_version}" PATHS "${CMAKE_PREFIX_PATH}" REQUIRED)
list(PREPEND CMAKE_MODULE_PATH "${DEPS_ROOT}/obs-studio-${obs_version}/cmake/finders")

function(install_libarary target)
  install(
    TARGETS ${target}
    DESTINATION data/obs-plugins/${CMAKE_PROJECT_NAME})

  install(
    FILES "$<TARGET_PDB_FILE:${target}>"
    CONFIGURATIONS RelWithDebInfo Debug
    DESTINATION data/obs-plugins/${CMAKE_PROJECT_NAME}
    OPTIONAL)
endfunction()

function(add_file target)
  install(FILES ${target} DESTINATION data/obs-plugins/${CMAKE_PROJECT_NAME})
endfunction()

function(parse_guid name guid)
  if(name STREQUAL "")
    message(FATAL_ERROR "no name supplied")
  endif()

  if(guid STREQUAL "")
    message(FATAL_ERROR "no GUID supplied")
  else()
    set(INVALID_GUID ON)

    string(REPLACE "-" ";" GUID_VALS ${guid})

    list(LENGTH GUID_VALS GUID_VAL_COUNT)
    if(GUID_VAL_COUNT EQUAL 5)
      string(REPLACE ";" "0" GUID_HEX ${GUID_VALS})
      string(REGEX MATCH "[0-9a-fA-F]+" GUID_ACTUAL_HEX ${GUID_HEX})
      if(GUID_ACTUAL_HEX STREQUAL GUID_HEX)
        list(GET GUID_VALS 0 GUID_VALS_DATA1)
        list(GET GUID_VALS 1 GUID_VALS_DATA2)
        list(GET GUID_VALS 2 GUID_VALS_DATA3)
        list(GET GUID_VALS 3 GUID_VALS_DATA4)
        list(GET GUID_VALS 4 GUID_VALS_DATA5)
        string(LENGTH ${GUID_VALS_DATA1} GUID_VALS_DATA1_LENGTH)
        string(LENGTH ${GUID_VALS_DATA2} GUID_VALS_DATA2_LENGTH)
        string(LENGTH ${GUID_VALS_DATA3} GUID_VALS_DATA3_LENGTH)
        string(LENGTH ${GUID_VALS_DATA4} GUID_VALS_DATA4_LENGTH)
        string(LENGTH ${GUID_VALS_DATA5} GUID_VALS_DATA5_LENGTH)
        if(GUID_VALS_DATA1_LENGTH EQUAL 8
           AND GUID_VALS_DATA2_LENGTH EQUAL 4
           AND GUID_VALS_DATA3_LENGTH EQUAL 4
           AND GUID_VALS_DATA4_LENGTH EQUAL 4
           AND GUID_VALS_DATA5_LENGTH EQUAL 12)
          set(${name}_GUID_VAL01 ${GUID_VALS_DATA1} PARENT_SCOPE)
          set(${name}_GUID_VAL02 ${GUID_VALS_DATA2} PARENT_SCOPE)
          set(${name}_GUID_VAL03 ${GUID_VALS_DATA3} PARENT_SCOPE)
          string(SUBSTRING ${GUID_VALS_DATA4} 0 2 GUID_VAL04)
          string(SUBSTRING ${GUID_VALS_DATA4} 2 2 GUID_VAL05)
          string(SUBSTRING ${GUID_VALS_DATA5} 0 2 GUID_VAL06)
          string(SUBSTRING ${GUID_VALS_DATA5} 2 2 GUID_VAL07)
          string(SUBSTRING ${GUID_VALS_DATA5} 4 2 GUID_VAL08)
          string(SUBSTRING ${GUID_VALS_DATA5} 6 2 GUID_VAL09)
          string(SUBSTRING ${GUID_VALS_DATA5} 8 2 GUID_VAL10)
          string(SUBSTRING ${GUID_VALS_DATA5} 10 2 GUID_VAL11)
          set(${name}_GUID_VAL04 ${GUID_VAL04} PARENT_SCOPE)
          set(${name}_GUID_VAL05 ${GUID_VAL05} PARENT_SCOPE)
          set(${name}_GUID_VAL06 ${GUID_VAL06} PARENT_SCOPE)
          set(${name}_GUID_VAL07 ${GUID_VAL07} PARENT_SCOPE)
          set(${name}_GUID_VAL08 ${GUID_VAL08} PARENT_SCOPE)
          set(${name}_GUID_VAL09 ${GUID_VAL09} PARENT_SCOPE)
          set(${name}_GUID_VAL10 ${GUID_VAL10} PARENT_SCOPE)
          set(${name}_GUID_VAL11 ${GUID_VAL11} PARENT_SCOPE)
          set(INVALID_GUID OFF)
        endif()
      endif()
    endif()
  endif()

  if(INVALID_GUID)
    message(FATAL_ERROR "invalid GUID supplied")
  endif()
endfunction()
