cmake_minimum_required(VERSION 2.8)
project(inip)

#Set build mode to release if no build-type was specified
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

#Debug postfix
set(CMAKE_DEBUG_POSTFIX "d")

#Add C++11 support
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#Add library modules
add_library(inip SHARED "${CMAKE_SOURCE_DIR}/inip.cpp")

#Installation
install(FILES "${CMAKE_SOURCE_DIR}/inip.h" DESTINATION "include/")

set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS inip DESTINATION ${INSTALL_DIR})
