cmake_minimum_required(VERSION 2.8)

project( assignment41 )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( assignment41 assignment41.cpp )
target_link_libraries( assignment41 ${OpenCV_LIBS} )
set_target_properties(assignment41 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../Data/")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT assignment41)