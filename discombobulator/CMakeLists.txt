# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the project name
PROJECT(discombobulator)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  ADD_DEFINITIONS("-D__LINUX__")
ENDIF()

FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})

# Some of the libraries have different names than their Find*.cmake name
SET(LIBRARIES_LINKED
  boost_locale
)

# Create this executable from these source files
ADD_EXECUTABLE(discombobulator main.cpp)

#need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES_LINKED})
