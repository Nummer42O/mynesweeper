## /main/ ##

cmake_minimum_required(VERSION 3.16)

find_package(Boost 1.71
  COMPONENTS log
  REQUIRED
)

target_sources(${_EXECUTABLE_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${_EXECUTABLE_NAME}
  PRIVATE
    ${Boost_LIBRARIES}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/visuals)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logic)