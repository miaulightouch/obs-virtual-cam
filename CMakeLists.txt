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

#
# Custom rules below this line
#

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers.cmake" NO_POLICY_SCOPE)

# Setup camera GUID
parse_guid(VirtualAudio "B750E5CD-5E7E-4ED3-B675-A5003C439997")
parse_guid(VirtualAudio2 "B750E5CD-5E7E-4ED3-B675-A5003C439998")
parse_guid(VirtualAudio3 "B750E5CD-5E7E-4ED3-B675-A5003C439999")
parse_guid(VirtualAudio4 "B750E5CD-5E7E-4ED3-B675-A5003C4399A0")
parse_guid(VirtualVideo "27B05C2D-93DC-474A-A5DA-9BBA34CB2A9C")
parse_guid(VirtualVideo2 "27B05C2D-93DC-474A-A5DA-9BBA34CB2A9D")
parse_guid(VirtualVideo3 "27B05C2D-93DC-474A-A5DA-9BBA34CB2A9E")
parse_guid(VirtualVideo4 "27B05C2D-93DC-474A-A5DA-9BBA34CB2A9F")
configure_file(${CMAKE_SOURCE_DIR}/src/virtualcam-guid.c.in ${CMAKE_CURRENT_BINARY_DIR}/virtualcam-guid.c @ONLY)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/virtualcam-guid.c)
add_library(virtualcam-guid STATIC)
target_sources(
  virtualcam-guid
  PRIVATE virtualcam-guid.c
  PUBLIC src/virtualcam-guid.h)
target_include_directories(virtualcam-guid PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE virtualcam-guid)

# only build the 64bit version plugin
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_sources(
    ${CMAKE_PROJECT_NAME}
    PRIVATE src/dshow-plugin.cpp
            src/tiny-nv12-scale.c
            src/tiny-nv12-scale.h
            src/shared-memory-queue.c
            src/shared-memory-queue.h
            src/virtualcam.c)

  set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
endif()

# build the dshow source(both 32bit/64bit)
add_subdirectory(src/virtualcam-module)
