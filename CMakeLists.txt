cmake_minimum_required(VERSION 3.8)

project(MatchThree C CXX)

add_subdirectory(thirdparty/raylib)

add_executable(game
    src/main.cpp
)
target_compile_features(game PRIVATE cxx_std_11)
target_link_libraries(game PRIVATE raylib_static)