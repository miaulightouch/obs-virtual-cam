cmake_minimum_required(VERSION 3.16...3.21)
project(obs-virtualcam VERSION 2.0.6)

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
add_subdirectory(src/virtual-source)

# Build virtual-output only on 64-bit systems
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  add_subdirectory(src/virtual-output)
endif()
