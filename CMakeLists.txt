﻿cmake_minimum_required(VERSION 3.25)
project(DirectXPlayground)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_library(DirectXPlayground SHARED ${SOURCES})

add_subdirectory(external/minhook)
add_subdirectory(external/kiero)
add_subdirectory(external/nuklear)

target_include_directories(DirectXPlayground PRIVATE external/minhook/include)
target_include_directories(DirectXPlayground PRIVATE external/kiero)
target_include_directories(DirectXPlayground PRIVATE external/nuklear)

target_link_libraries(DirectXPlayground PRIVATE minhook)