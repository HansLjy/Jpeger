cmake_minimum_required(VERSION 3.22)
project(Jpeger)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# debug mode
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall")
set(CMAKE_LINK_LIBRARY_FLAG_DEBUG "-g")

# release mode
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(cmake/dependency.cmake)

if(BUILD_TEST)
    include(cmake/test-dependency.cmake)
endif()

add_definitions(-DCONFIG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/config")
add_definitions(-DIMAGE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/image")
add_subdirectory(src)