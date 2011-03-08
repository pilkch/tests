# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the project name
PROJECT(source_cleaner)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  ADD_DEFINITIONS("-D__LINUX__")
ENDIF()

# Add executable called "source_cleaner" that is built from the source file
# "main.cpp". The extensions are automatically found.
ADD_EXECUTABLE(source_cleaner main.cpp)

