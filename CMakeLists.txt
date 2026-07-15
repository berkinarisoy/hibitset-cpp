cmake_minimum_required(VERSION 3.23)
project(hibitset VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(hibitset INTERFACE)
add_library(hibitset::hibitset ALIAS hibitset)

target_include_directories(hibitset INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(hibitset INTERFACE cxx_std_20)

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(benchmarks)
endif()