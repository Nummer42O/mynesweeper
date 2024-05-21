## /main/visual/widgets/ ##

cmake_minimum_required(VERSION 3.16)

target_sources(${_EXECUTABLE_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tile.cpp
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)