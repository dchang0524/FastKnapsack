cmake_minimum_required(VERSION 3.10)
project(FastKnapsack LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tell CMake where to find our headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Gather all implementation files
file(GLOB CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# Build a static library "core" from them
add_library(core STATIC ${CORE_SOURCES})

# Build the final executable from main.cpp
add_executable(knapsack_solver
    ${PROJECT_SOURCE_DIR}/main.cpp
)

# Link in the core library
target_link_libraries(knapsack_solver PRIVATE core)
