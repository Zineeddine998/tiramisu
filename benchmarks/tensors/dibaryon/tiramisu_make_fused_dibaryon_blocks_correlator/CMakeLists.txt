cmake_minimum_required(VERSION 3.5)
project(Autoscheduler)

include(CMakePrintHelpers)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_VERBOSE_MAKEFILE "ON")
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
SET(BUILD_TARGET "g")
SET(FUNCTION_TO_BUILD "tiramisu_make_fused_dibaryon_blocks_correlator")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl -g -fno-rtti -lz -lpthread -std=c++17 -O0")


include_directories($ENV{TIRAMISU_ROOT}/3rdParty/Halide/install/include $ENV{TIRAMISU_ROOT}/include/ $ENV{TIRAMISU_ROOT}/3rdParty/isl/include)

link_directories($ENV{TIRAMISU_ROOT}/build $ENV{TIRAMISU_ROOT}/3rdParty/Halide/install/lib64 $ENV{TIRAMISU_ROOT}/3rdParty/isl/build/lib)


cmake_print_variables(BUILD_TARGET)
if (BUILD_TARGET STREQUAL "a")
    set(generator_SOURCES ${FUNCTION_TO_BUILD}_autoscheduler.cpp)
elseif(BUILD_TARGET STREQUAL "g")
    set(generator_SOURCES ${FUNCTION_TO_BUILD}_generator.cpp)
endif()

add_executable(generator ${generator_SOURCES})
 
#target_link_libraries(generator tiramisu tiramisu_auto_scheduler)
target_link_libraries(generator tiramisu tiramisu_auto_scheduler Halide isl)
