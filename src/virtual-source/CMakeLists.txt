cmake_minimum_required(VERSION 3.16...3.26)

project(obs-virtualsource VERSION ${_version})
add_library(${PROJECT_NAME} MODULE)

find_package(FFmpeg REQUIRED COMPONENTS avutil swscale avfilter avcodec avformat swresample)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::avutil FFmpeg::swscale)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_arch x64)
  set(bit 64bit)
  add_file(${FFmpeg_avfilter_LIBRARY} ${bit})
  add_file(${FFmpeg_avcodec_LIBRARY} ${bit})
  add_file(${FFmpeg_avformat_LIBRARY} ${bit})
  add_file(${FFmpeg_swresample_LIBRARY} ${bit})
else()
  set(_arch x86)
  set(bit 32bit)
endif()

add_file(${FFmpeg_avutil_LIBRARY} ${bit})
add_file(${FFmpeg_swscale_LIBRARY} ${bit})

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
          ../queue/share_queue_read.h
          ../queue/share_queue_read.cpp
          ../queue/share_queue.h)

configure_file(${CMAKE_SOURCE_DIR}/cmake/windows/resources/resource.rc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc)

install_libarary(${PROJECT_NAME} ${bit})
