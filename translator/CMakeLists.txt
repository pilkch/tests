# Set the minimum cmake version
cmake_minimum_required(VERSION 2.6)

# Set the project name
project(translator)

# Add executable called "translator" that is built from the source file "main.cpp". The extensions are automatically found.
add_executable(translator main.cpp)

set_property(TARGET translator PROPERTY CXX_STANDARD 14)

