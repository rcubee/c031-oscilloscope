cmake_minimum_required(VERSION 3.16)

project(shared VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(c031-oscilloscope-shared STATIC)

target_sources(c031-oscilloscope-shared PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/osc_shared.c
)

target_include_directories(c031-oscilloscope-shared PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include/
)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()
