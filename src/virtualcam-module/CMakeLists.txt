cmake_minimum_required(VERSION 3.16...3.26)

project(obs-virtualcam-module VERSION ${_version})
add_library(${PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OBS::libobs)

# import subset of libdshowcapture
add_library(libdshowcapture INTERFACE)
add_library(OBS::libdshowcapture ALIAS libdshowcapture)
string(
  JSON
  libdshowcapture_version
  GET
  ${buildspec}
  "dependencies"
  "libdshowcapture"
  "version")
set(libdshowcapture_root "${DEPS_ROOT}/libdshowcapture-${libdshowcapture_version}")
target_include_directories(libdshowcapture INTERFACE "${libdshowcapture_root}")
target_sources(
  libdshowcapture
  INTERFACE ${libdshowcapture_root}/dshowcapture.hpp
            ${libdshowcapture_root}/source/dshow-base.cpp
            ${libdshowcapture_root}/source/dshow-base.hpp
            ${libdshowcapture_root}/source/dshow-enum.cpp
            ${libdshowcapture_root}/source/dshow-enum.hpp
            ${libdshowcapture_root}/source/dshow-formats.cpp
            ${libdshowcapture_root}/source/dshow-formats.hpp
            ${libdshowcapture_root}/source/dshow-media-type.cpp
            ${libdshowcapture_root}/source/dshow-media-type.hpp
            ${libdshowcapture_root}/source/log.cpp
            ${libdshowcapture_root}/source/log.hpp
            ${libdshowcapture_root}/source/output-filter.cpp
            ${libdshowcapture_root}/source/output-filter.hpp)
target_compile_definitions(libdshowcapture INTERFACE _UP_WINDOWS=1)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_output_suffix "64")
else()
  set(_output_suffix "32")
endif()

target_sources(
  ${PROJECT_NAME}
  PRIVATE sleepto.c
          sleepto.h
          placeholder.cpp
          virtualcam-module.cpp
          virtualcam-filter.cpp
          virtualcam-filter.hpp
          virtualcam-module.rc
          ../shared-memory-queue.c
          ../shared-memory-queue.h
          ../tiny-nv12-scale.c
          ../tiny-nv12-scale.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(MODULE_DESCRIPTION "OBS Virtual Camera output module")

configure_file(${CMAKE_SOURCE_DIR}/cmake/windows/resources/resource.rc.in virtualcam-module.rc)
target_sources(${PROJECT_NAME} PRIVATE virtualcam-module.rc)

target_link_libraries(${PROJECT_NAME} PRIVATE OBS::libdshowcapture setupapi winmm strmiids gdiplus virtualcam-guid)
target_link_options(${PROJECT_NAME} PRIVATE "LINKER:/ignore:4104")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/virtualcam-module.def.in ${CMAKE_CURRENT_BINARY_DIR}/virtualcam-module.def)
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/virtualcam-module.def)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}${_output_suffix}")

install_libarary(${PROJECT_NAME})
