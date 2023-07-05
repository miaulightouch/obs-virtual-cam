cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_qt(COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

###### Custom rules below this line

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake" NO_POLICY_SCOPE)

find_package(FFmpeg REQUIRED COMPONENTS avfilter avutil)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE FFmpeg::avfilter FFmpeg::avutil)

# only build the 64bit version plugin
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_sources(
    ${CMAKE_PROJECT_NAME}
    PRIVATE src/virtual_main.cpp
            src/virtual_output.h
            src/virtual_output.cpp
            src/virtual_properties.h
            src/virtual_properties.cpp
            src/virtual_filter.h
            src/virtual_filter.cpp
            src/hflip.h
            src/hflip.cpp
            src/queue/share_queue_write.h
            src/queue/share_queue_write.cpp
            src/get_format.h
            src/queue/share_queue.h)

  set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
endif()

# build the dshow source(both 32bit/64bit)
add_subdirectory(src/virtual-source)
