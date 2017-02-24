# Set the minimum cmake version
cmake_minimum_required(VERSION 2.6)

# Set the project name
project(stopwatch)

# Add executable called "stopwatch" that is built from the source file "main.cpp". The extensions are automatically found.
add_executable(stopwatch main.cpp)

set_property(TARGET stopwatch PROPERTY CXX_STANDARD 14)

