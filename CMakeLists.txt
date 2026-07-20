cmake_minimum_required(VERSION 3.10)
option(HASHMAP_TEST "Enable hashmap test. To Enable hashmap benchmark enable O3 optimisation" OFF)

project(hashmap_c LANGUAGES C)
add_library(hashmap_c STATIC "hashmap.c")
target_include_directories(hashmap_c PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(HASHMAP_TEST)
  target_compile_definitions(hashmap.c PRIVATE HASHMAP_TEST)
endif()
