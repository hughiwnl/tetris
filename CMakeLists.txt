cmake_minimum_required(VERSION 3.20)
project(Tetris CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(tetris
    src/main.cpp
    src/game.cpp
    src/board.cpp
    src/tetromino.cpp
    src/renderer.cpp
    src/input.cpp
)

target_include_directories(tetris PRIVATE src)
target_link_libraries(tetris PRIVATE SFML::Graphics SFML::Window SFML::System)
