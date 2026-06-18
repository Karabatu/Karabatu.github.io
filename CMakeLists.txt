cmake_minimum_required(VERSION 3.10)
project(PhysicsGasEngine)

set(CMAKE_CXX_STANDARD 17)

# Source files
set(SOURCES
    src/main.cpp
    src/Simulation_Part1.cpp
    src/Simulation_Part2.cpp
)

# Executable
add_executable(PhysicsGasEngine ${SOURCES})
