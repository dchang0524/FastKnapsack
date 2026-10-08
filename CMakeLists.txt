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

add_executable(coinchange_solver
    ${PROJECT_SOURCE_DIR}/coinchange_benchmark/coinchange_simplified_solver.cpp
)
target_link_libraries(coinchange_solver PRIVATE core)

add_executable(knapsack_solver
    ${PROJECT_SOURCE_DIR}/knapsack_benchmark/knapsack.cpp
)
target_link_libraries(knapsack_solver PRIVATE core)

add_executable(coinchange_traditional
    ${PROJECT_SOURCE_DIR}/coinchange_benchmark/coinchange_traditional.cpp
)

add_executable(knapsack_traditional
    ${PROJECT_SOURCE_DIR}/knapsack_benchmark/traditional_knapsack.cpp
)

enable_testing()
add_executable(algorithm4_test ${PROJECT_SOURCE_DIR}/tests/algorithm4_test.cpp)
target_link_libraries(algorithm4_test PRIVATE core)
add_test(NAME algorithm4_test COMMAND algorithm4_test)

add_executable(probability_space_test
    ${PROJECT_SOURCE_DIR}/tests/probability_space_test.cpp)
target_link_libraries(probability_space_test PRIVATE core)
add_test(NAME probability_space_test COMMAND probability_space_test)

add_executable(deterministic_witness_test
    ${PROJECT_SOURCE_DIR}/tests/deterministic_witness_test.cpp)
target_link_libraries(deterministic_witness_test PRIVATE core)
add_test(NAME deterministic_witness_test COMMAND deterministic_witness_test)

add_executable(remaining_algorithms_test
    ${PROJECT_SOURCE_DIR}/tests/remaining_algorithms_test.cpp)
target_link_libraries(remaining_algorithms_test PRIVATE core)
add_test(NAME remaining_algorithms_test COMMAND remaining_algorithms_test)

add_executable(optimized_peeling_test
    ${PROJECT_SOURCE_DIR}/tests/optimized_peeling_test.cpp)
target_link_libraries(optimized_peeling_test PRIVATE core)
add_test(NAME optimized_peeling_test COMMAND optimized_peeling_test)

add_executable(witness_profile ${PROJECT_SOURCE_DIR}/tests/witness_profile.cpp)
target_link_libraries(witness_profile PRIVATE core)
