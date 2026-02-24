cmake_minimum_required(VERSION 3.14)

set(projectName "grocery_store")

project(${projectName} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Cadmium is header-only; this points to wherever it's installed
# Default install path from the Cadmium build script is $HOME/cadmium_v2
if(NOT DEFINED ENV{CADMIUM})
    set(CADMIUM_PATH "$ENV{HOME}/cadmium_v2")
else()
    set(CADMIUM_PATH "$ENV{CADMIUM}")
endif()

include_directories(${CADMIUM_PATH}/include)

add_subdirectory(main)

