## /main/visuals/ ##

cmake_minimum_required(VERSION 3.16)

pkg_check_modules(
  GTKMM
  IMPORTED_TARGET
    gtkmm-3.0
    glibmm-2.4
  REQUIRED
)

target_sources(${_EXECUTABLE_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${GTKMM_INCLUDE_DIRS}
)
target_link_libraries(${_EXECUTABLE_NAME}
  PRIVATE
    ${GTKMM_LIBRARIES}
)

add_subdirectory(dialogs)
add_subdirectory(widgets)