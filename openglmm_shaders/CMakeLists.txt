# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the project name
PROJECT(openglmm_shaders)

SET(CMAKE_SOURCE_DIR ./../../library/)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

SET(PROJECT_DIRECTORY "./")

SET(LIBRARY_INCLUDE "${CMAKE_SOURCE_DIR}/include/")
SET(LIBRARY_SRC "${CMAKE_SOURCE_DIR}/src/")

# Debug build
SET(CMAKE_BUILD_TYPE Debug)
ADD_DEFINITIONS("-DDEBUG")

# Release build
#SET(CMAKE_BUILD_TYPE Release)
#ADD_DEFINITIONS("-DNDEBUG")


# Select flags
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

# Add definitions, compiler switches, etc.
SET(CMAKE_CXX_FLAGS "-Wall -W -Wextra -Werror -Wformat -Wformat-y2k -Winit-self -Wstack-protector -Wunknown-pragmas -Wundef -Wwrite-strings -Wno-unused-parameter -Wpointer-arith -Wno-switch -Woverloaded-virtual -Wno-stack-protector -Wnon-virtual-dtor -Wmissing-include-dirs -Wuninitialized -Wc++0x-compat -std=c++0x -ffor-scope")


IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  ADD_DEFINITIONS("-D__LINUX__")
ENDIF()

IF(WIN32)
  ADD_DEFINITIONS("-DUNICODE -D_UNICODE")
  ADD_DEFINITIONS("-DNOMINMAX")
ENDIF()

ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_OPENGL_VERSION=330")
ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_OPENGL_STRICT")
ADD_DEFINITIONS("-DBUILD_LIBOPENGLMM_WINDOW_SDL")

INCLUDE_DIRECTORIES(${LIBRARY_INCLUDE})


# Files from library directory

MACRO(PREFIX_PATHS SOURCE_DIRECTORY SOURCE_FILES)
MESSAGE(STATUS "PREFIX_PATHS dir=${SOURCE_DIRECTORY}, files=${SOURCE_FILES}, ${ARGN}")
MESSAGE(a=${SOURCE_FILES})
  SET(OUTPUT_FILES "")
  SET(a ${OUTPUT_FILES})
  SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILES}")
  FOREACH(SOURCE_FILE ${ARGN})
    MESSAGE(b=${SOURCE_FILE})
    SET(a ${OUTPUT_FILES})
    SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILE}")
  ENDFOREACH()
MESSAGE(d=${OUTPUT_FILES})
ENDMACRO(PREFIX_PATHS)




SET(LIBRARY_LIBVOODOOMM_SOURCE_DIRECTORY libvoodoomm/)
SET(LIBRARY_LIBVOODOOMM_SOURCE_FILES
libvoodoomm.cpp cImage.cpp)

IF(WIN32)
  SET(LIBRARY_LIBVOODOOMM_SOURCE_FILES ${LIBRARY_LIBVOODOOMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBVOODOOMM_SOURCE_DIRECTORY} ${LIBRARY_LIBVOODOOMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBVOODOOMM_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_LIBOPENGLMM_SOURCE_DIRECTORY libopenglmm/)
SET(LIBRARY_LIBOPENGLMM_SOURCE_FILES
libopenglmm.cpp cContext.cpp cGeometry.cpp cShader.cpp cSystem.cpp cTexture.cpp cVertexBufferObject.cpp cWindow.cpp)

IF(WIN32)
  SET(LIBRARY_LIBOPENGLMM_SOURCE_FILES ${LIBRARY_LIBOPENGLMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBOPENGLMM_SOURCE_DIRECTORY} ${LIBRARY_LIBOPENGLMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBOPENGLMM_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY libtrashmm/)
SET(LIBRARY_LIBTRASHMM_SOURCE_FILES
libtrashmm.cpp)

PREFIX_PATHS(${LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY} ${LIBRARY_LIBTRASHMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_LIBXDGMM_SOURCE_DIRECTORY libxdgmm/)
SET(LIBRARY_LIBXDGMM_SOURCE_FILES
libxdgmm.cpp)

IF(WIN32)
  SET(LIBRARY_LIBXDGMM_SOURCE_FILES ${LIBRARY_LIBXDGMM_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_LIBXDGMM_SOURCE_DIRECTORY} ${LIBRARY_LIBXDGMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_SPITFIRE_SOURCE_DIRECTORY spitfire/)
SET(LIBRARY_SPITFIRE_SOURCE_FILES
spitfire.cpp
math/cVec2.cpp math/cVec3.cpp math/cVec4.cpp math/cMat3.cpp math/cMat4.cpp math/cQuaternion.cpp math/math.cpp math/cColour.cpp
storage/file.cpp storage/filesystem.cpp
util/datetime.cpp util/string.cpp util/thread.cpp)

IF(WIN32)
  SET(LIBRARY_SPITFIRE_SOURCE_FILES ${LIBRARY_SPITFIRE_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_SPITFIRE_SOURCE_DIRECTORY} ${LIBRARY_SPITFIRE_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES ${OUTPUT_FILES})


SET(LIBRARY_BREATHE_SOURCE_DIRECTORY breathe/)
SET(LIBRARY_BREATHE_SOURCE_FILES
render/model/cFileFormatOBJ.cpp render/model/cStatic.cpp)

IF(WIN32)
  SET(LIBRARY_BREATHE_SOURCE_FILES ${LIBRARY_BREATHE_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_BREATHE_SOURCE_DIRECTORY} ${LIBRARY_BREATHE_SOURCE_FILES})
SET(OUTPUT_LIBRARY_BREATHE_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_SOURCE_FILES ${OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES} ${OUTPUT_LIBRARY_BREATHE_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBVOODOOMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBOPENGLMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES}
GL/GLee.cpp
)
PREFIX_PATHS(${LIBRARY_SRC} ${LIBRARY_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SOURCE_FILES ${OUTPUT_FILES})
#MESSAGE(final=${OUTPUT_LIBRARY_SOURCE_FILES})

#list all source files here
ADD_EXECUTABLE(${PROJECT_NAME} main.cpp ${OUTPUT_LIBRARY_SOURCE_FILES} ${PROJECT_SOURCE_FILES})


FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})


#SET(LIBRARIES
#  OpenGL
#  SDL
#  SDL_image
#  SDL_ttf
#)


# Some of the libraries have different names than their Find*.cmake name
SET(LIBRARIES_LINKED
  boost_date_time
  boost_filesystem
  boost_iostreams
  boost_locale
  boost_system
  GL
  GLU
  SDL
  SDL_image
  SDL_ttf
  xdg-basedir
)


#FOREACH(LIBRARY_FILE ${LIBRARIES})
#  Find_Package(${LIBRARY_FILE} REQUIRED)
#ENDFOREACH(LIBRARY_FILE)

#need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES_LINKED})
