## /main/visual/dialogs/ ##

cmake_minimum_required(VERSION 3.16)

target_sources(${_EXECUTABLE_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/new_game_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/no_moves_left_dialog.cpp
)
target_include_directories(${_EXECUTABLE_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)