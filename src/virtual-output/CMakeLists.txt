cmake_minimum_required(VERSION 3.16...3.21)

# Change obs-plugintemplate to your plugin's name in a machine-readable format (e.g.:
# obs-myawesomeplugin) and set
project(obs-virtualoutput)
add_library(${PROJECT_NAME} MODULE)
add_dependencies(${PROJECT_NAME} obs-virtualsource)
find_package(
  FFmpeg
  COMPONENTS avutil avfilter
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
set(virtualoutput_SOURCES virtual_main.cpp virtual_output.cpp virtual_properties.cpp
                          virtual_filter.cpp hflip.cpp ../queue/share_queue_write.cpp)

set(virtualoutput_HEADERS
    get_format.h
    virtual_output.h
    virtual_properties.h
    virtual_filter.h
    hflip.h
    ../queue/share_queue_write.h
    ../queue/share_queue.h)

target_sources(${PROJECT_NAME} PRIVATE ${virtualoutput_SOURCES} ${virtualoutput_HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /d2:-FH4-)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /PDBSTRIPPED /d2:-FH4-)
else()
  target_link_options(${PROJECT_NAME} PRIVATE /OPT:REF /OPT:ICF /d2:-FH4-)
endif()

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(../../cmake/ObsPluginHelpers.cmake)

# Uncomment these lines if you want to use the OBS Frontend API in your plugin
find_package(obs-frontend-api REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OBS::obs-frontend-api)

# Uncomment those lines if you want to use Qt in your plugin
find_qt(COMPONENTS Widgets Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets)
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/installer-Windows.nsi.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.nsi @ONLY)

  configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/resource.rc.in
                 ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)
  target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)

  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  endif()
endif()

# --- End of section ---

setup_plugin_target(${PROJECT_NAME})
