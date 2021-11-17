cmake_minimum_required(VERSION 3.13)
project(pljit)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Infrastructure)

add_subdirectory(pljit)
add_subdirectory(test)
add_subdirectory(scripts)