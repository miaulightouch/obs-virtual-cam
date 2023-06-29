cmake_minimum_required(VERSION 3.16...3.21)

# Change obs-plugintemplate to your plugin's name in a machine-readable format (e.g.:
# obs-myawesomeplugin) and set
project(obs-virtualsource)
add_library(${PROJECT_NAME} MODULE)
find_package(
  FFmpeg
  COMPONENTS avutil avfilter swscale
  REQUIRED)

# Replace `Your Name Here` with the name (yours or your organization's) you want to see as the
# author of the plugin (in the plugin's metadata itself and in the installers)
set(PLUGIN_AUTHOR "Miau Lightouch")

# Replace 'https://www.example.com` with a link to the website of your plugin or repository
set(PLUGIN_WEBSITE "https://www.example.com")

# Replace `com.example.obs-plugin-template` with a unique Bundle ID for macOS releases (used both in
# the installer and when submitting the installer for notarization) set(MACOS_BUNDLEID
# "com.example.${PROJECT_NAME}")

# Replace `me@contoso.com` with the maintainer email address you want to put in Linux packages
# set(LINUX_MAINTAINER_EMAIL "miau@example.com")

# Add your custom source files here - header files are optional and only required for visibility
# e.g. in Xcode or Visual Studio
set(virtualsource_SOURCES dllmain.cpp virtual-cam.cpp virtual-audio.cpp clock.cpp
                          virtual-source.def ../queue/share_queue_read.cpp)

set(virtualsource_HEADERS virtual-cam.h virtual-audio.h clock.h ../queue/share_queue_read.h
                          ../queue/share_queue.h)

target_sources(${PROJECT_NAME} PRIVATE ${virtualsource_SOURCES} ${virtualsource_HEADERS})
include_directories("../../dshowdeps/include")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  find_library(
    DShowLib strmbase
    PATHS "../../dshowdeps/lib/x64"
    NO_DEFAULT_PATH)
  find_library(
    DShowLibD strmbasd
    PATHS "../../dshowdeps/lib/x64"
    NO_DEFAULT_PATH)
else()
  find_library(
    DShowLib strmbase
    PATHS "../../dshowdeps/lib/x86"
    NO_DEFAULT_PATH)
  find_library(
    DShowLibD strmbasd
    PATHS "../../dshowdeps/lib/x86"
    NO_DEFAULT_PATH)
endif()

set(DSHOW_LIBRARY optimized ${DShowLib} debug ${DShowLibD})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES} ${DSHOW_LIBRARY} winmm Strmiids)
target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099 /DEF:virtual-source.def
                    /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /PDBSTRIPPED)
else()
  target_link_options(${PROJECT_NAME} PRIVATE /OPT:REF)
endif()

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(../../cmake/ObsPluginHelpers.cmake)

# Uncomment these lines if you want to use the OBS Frontend API in your plugin
# find_package(obs-frontend-api REQUIRED) target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
# OBS::obs-frontend-api)

# Uncomment those lines if you want to use Qt in your plugin find_qt(COMPONENTS Widgets Core)
# target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets) set_target_properties(
# ${CMAKE_PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/resource.rc.in
                 ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)
  target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)

  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  endif()
endif()

# --- End of section ---

function(setup_library_target_for_testing target)
  install(
    FILES $<TARGET_FILE:${target}>
    DESTINATION $<CONFIG>/${OBS_LIBRARY_DESTINATION}
    COMPONENT obs_testing
    EXCLUDE_FROM_ALL)

  install(
    FILES $<TARGET_PDB_FILE:${target}>
    CONFIGURATIONS "RelWithDebInfo" "Debug"
    DESTINATION $<CONFIG>/${OBS_LIBRARY_DESTINATION}
    COMPONENT obs_testing
    OPTIONAL EXCLUDE_FROM_ALL)

  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -DCMAKE_INSTALL_PREFIX=${OBS_BUILD_DIR}/rundir
      -DCMAKE_INSTALL_COMPONENT=obs_testing -DCMAKE_INSTALL_CONFIG_NAME=$<CONFIG> -P
      ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
    COMMENT "Installing to OBS test directory"
    VERBATIM)

endfunction()

function(setup_library_target target)
  # Set install directories
  install(
    TARGETS ${target}
    RUNTIME DESTINATION "${OBS_LIBRARY_DESTINATION}" COMPONENT ${target}_Runtime
    LIBRARY DESTINATION "${OBS_LIBRARY_DESTINATION}"
            COMPONENT ${target}_Runtime
            NAMELINK_COMPONENT ${target}_Development)

  # Set rundir install directory
  install(
    FILES $<TARGET_FILE:${target}>
    DESTINATION $<CONFIG>/${OBS_LIBRARY_DESTINATION}
    COMPONENT obs_rundir
    EXCLUDE_FROM_ALL)

  # Set install directory for optional PDB symbol files
  install(
    FILES $<TARGET_PDB_FILE:${target}>
    CONFIGURATIONS "RelWithDebInfo" "Debug"
    DESTINATION ${OBS_LIBRARY_DESTINATION}
    COMPONENT ${target}_Runtime
    OPTIONAL)

  # Set rundir install directory for optional PDB symbol files
  install(
    FILES $<TARGET_PDB_FILE:${target}>
    CONFIGURATIONS "RelWithDebInfo" "Debug"
    DESTINATION $<CONFIG>/${OBS_LIBRARY_DESTINATION}
    COMPONENT obs_rundir
    OPTIONAL EXCLUDE_FROM_ALL)

  if(DEFINED OBS_BUILD_DIR)
    setup_library_target_for_testing(${target})
  endif()

endfunction()

setup_library_target(${PROJECT_NAME})
