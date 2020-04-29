cmake_minimum_required(VERSION 3.8)

project(MatchThree CXX)

add_executable(game
    src/main.cpp
)
target_compile_features(game PRIVATE cxx_std_11)