cmake_minimum_required (VERSION 2.8)

# This is the name of the project
PROJECT( handDetect  )

# This is for recursively searching for other directories.
# add_subdirectory (DirectoryName)

# sources to include
include_directories (.)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
FIND_PACKAGE( OpenCV REQUIRED )
ADD_EXECUTABLE( handDetect  main.cpp)
TARGET_LINK_LIBRARIES( handDetect ${OpenCV_LIBS} )

