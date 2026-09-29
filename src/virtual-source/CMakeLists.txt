cmake_minimum_required(VERSION 3.16...3.26)

project(obs-virtualsource VERSION ${_version})
add_library(${PROJECT_NAME} MODULE)

find_package(FFmpeg REQUIRED COMPONENTS avutil swscale)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::swscale delayimp)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_arch x64)
  set(bit 64bit)
else()
  set(_arch x86)
  set(bit 32bit)
endif()

# FFmpeg is delay-loaded from a private folder (see ffmpeg-loader.h), because the OBS updater deletes FFmpeg DLLs with
# these names from bin/${bit}.
get_filename_component(_swscale_dll "${FFmpeg_swscale_LIBRARY}" NAME)
target_link_options(${PROJECT_NAME} PRIVATE /DELAYLOAD:${_swscale_dll})
add_file(${FFmpeg_avutil_LIBRARY} ${bit}/obs-virtualcam)
add_file(${FFmpeg_swscale_LIBRARY} ${bit}/obs-virtualcam)

# Setup dshowlib
include_directories("${CMAKE_SOURCE_DIR}/deps/include")
find_library(
  DShowLib strmbase
  PATHS "${CMAKE_SOURCE_DIR}/deps/lib/${_arch}"
  NO_DEFAULT_PATH)
find_library(
  DShowLibD strmbasd
  PATHS "${CMAKE_SOURCE_DIR}/deps/lib/${_arch}"
  NO_DEFAULT_PATH)
set(DSHOW_LIBRARY optimized ${DShowLib} debug ${DShowLibD})

# setup compile options
target_link_libraries(${PROJECT_NAME} PRIVATE ${DSHOW_LIBRARY} winmm Strmiids)
target_link_options(${PROJECT_NAME} PRIVATE /ignore:4099 /NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT /SAFESEH:NO)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)

target_sources(
  ${PROJECT_NAME}
  PRIVATE dllmain.cpp
          virtual-cam.h
          virtual-cam.cpp
          virtual-audio.h
          virtual-audio.cpp
          virtual-source.def
          clock.h
          clock.cpp
          ffmpeg-loader.h
          ffmpeg-loader.cpp
          ../queue/share_queue_read.h
          ../queue/share_queue_read.cpp
          ../queue/share_queue.h)

configure_file(${CMAKE_SOURCE_DIR}/cmake/windows/resources/resource.rc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)

install_libarary(${PROJECT_NAME} ${bit})
