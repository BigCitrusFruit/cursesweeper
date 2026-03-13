cmake_minimum_required(VERSION 3.10)
project(minesweeper)

find_package(Curses)
if(Curses_FOUND AND NOT TARGET Curses::Curses)
  add_library(Curses::Curses INTERFACE IMPORTED)
  set_target_properties(
    Curses::Curses
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${CURSES_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CURSES_INCLUDE_DIRS}"
  )
endif()

add_executable(minesweeper minesweeper.c)
target_link_libraries(minesweeper PRIVATE Curses::Curses)
