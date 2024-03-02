cmake_minimum_required(VERSION 3.10)
project(minesweeper)

# set standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include(GNUInstallDirs)

# custom
set(_INSTALL_SPRITE_DIRECTORY "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}")
set(_PROGRAM_NAME "Mynesweeper")
string(TOLOWER ${_PROGRAM_NAME} _EXECUTABLE_NAME)
set(_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/games")
if(${CMAKE_BUILD_TYPE} MATCHES "^(Debug|RelWithDebInfo)$")
  set(_DEBUG_DEFINITION "MW_DEBUG")
  set(_NEEDS_TERMINAL "true")
else()
  set(_DEBUG_DEFINITION "")
  set(_NEEDS_TERMINAL "false")
endif()

# use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig 0.29 REQUIRED)
pkg_check_modules(
  GTKMM
  IMPORTED_TARGET
    gtkmm-3.0
    glibmm-2.4
  REQUIRED
)

find_package(Boost 1.71
  COMPONENTS log
  REQUIRED
)

# build
add_executable(${_EXECUTABLE_NAME}
  # graphics
  ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/new_game_dialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/no_moves_left_dialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tile.cpp
  # logic
  ${CMAKE_CURRENT_SOURCE_DIR}/src/logic.cpp
  # main application
  ${CMAKE_CURRENT_SOURCE_DIR}/src/application.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
target_link_libraries(${_EXECUTABLE_NAME}
  PRIVATE
    ${GTKMM_LIBRARIES}
    ${Boost_LIBRARIES}
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
)
target_compile_definitions(${_EXECUTABLE_NAME}
  PRIVATE
    SPRITE_DIRECTORY="$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sprites>$<INSTALL_INTERFACE:${_INSTALL_SPRITE_DIRECTORY}>"
    PROGRAM_NAME="${_PROGRAM_NAME}"
    EXECUTABLE_NAME="${_EXECUTABLE_NAME}"
    ${_DEBUG_DEFINITION}
)

# configure
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${_EXECUTABLE_NAME}.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${_EXECUTABLE_NAME}.desktop"
  @ONLY
)

# install
install(
  TARGETS ${_EXECUTABLE_NAME}
  RUNTIME
    DESTINATION ${_INSTALL_DIR}
)
install(
  # NOTE: Do not remove the trailing slash; see https://cmake.org/cmake/help/v3.16/command/install.html#directory (paragraph 1)
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sprites/"
  DESTINATION ${_INSTALL_SPRITE_DIRECTORY}
  FILES_MATCHING
    PATTERN "*.bmp"
)
install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/sprites/icon.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/
  RENAME ${_EXECUTABLE_NAME}.png
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${_EXECUTABLE_NAME}.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
  PERMISSIONS
    OWNER_WRITE GROUP_WRITE
    WORLD_READ
    WORLD_EXECUTE
)


# uninstall target
# from: https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
if(NOT TARGET uninstall)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    IMMEDIATE @ONLY
  )

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  )
endif()
