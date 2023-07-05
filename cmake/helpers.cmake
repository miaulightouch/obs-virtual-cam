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

function(install_libarary target bit)
  install(
    TARGETS ${target}
    RUNTIME DESTINATION bin/${bit}
    LIBRARY DESTINATION bin/${bit})

  install(
    FILES "$<TARGET_PDB_FILE:${target}>"
    CONFIGURATIONS RelWithDebInfo Debug
    DESTINATION bin/${bit}
    OPTIONAL)
endfunction()

function(add_file target bit)
  install(FILES ${target} DESTINATION bin/${bit})
endfunction()
