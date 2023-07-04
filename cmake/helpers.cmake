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

function(install_libarary target arch)
  install(
    TARGETS ${target}
    RUNTIME DESTINATION bin/${arch}
    LIBRARY DESTINATION data/obs-plugins/${CMAKE_PROJECT_NAME})

  install(
    FILES "$<TARGET_PDB_FILE:${target}>"
    CONFIGURATIONS RelWithDebInfo Debug
    DESTINATION data/obs-plugins/${CMAKE_PROJECT_NAME}
    OPTIONAL)
endfunction()
