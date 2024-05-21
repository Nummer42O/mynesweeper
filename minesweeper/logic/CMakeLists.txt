## /main/logic/ ##

cmake_minimum_required(VERSION 3.16)

pkg_check_modules(
  SIGCPP
  IMPORTED_TARGET
    glibmm-2.4
  REQUIRED
)

target_sources(${_EXECUTABLE_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logic.cpp
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SIGCPP_INCLUDE_DIRS}
)
target_link_libraries(${_EXECUTABLE_NAME}
  PRIVATE
    ${SIGCPP_LIBRARIES}
)
