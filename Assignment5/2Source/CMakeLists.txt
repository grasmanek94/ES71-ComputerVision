cmake_minimum_required (VERSION 2.8)

# This is the name of the project
PROJECT( stitching  )

# This is for recursively searching for other directories.
# add_subdirectory (DirectoryName)

# sources to include
include_directories (. ${OpenCV_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
FIND_PACKAGE( OpenCV REQUIRED )
ADD_EXECUTABLE( stitching  main.cpp)
TARGET_LINK_LIBRARIES( stitching ${OpenCV_LIBS} )
set_target_properties(stitching PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../2Build/")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT stitching)

