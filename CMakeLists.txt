cmake_minimum_required(VERSION 3.8)

project(MatchThree C CXX)

add_subdirectory(thirdparty/raylib)

add_executable(game
    src/list.h

    src/main.cpp
)
if(PLATFORM STREQUAL "Web")
target_compile_options(game PRIVATE -std=c++11)
target_link_options(game PRIVATE -s USE_GLFW=3 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/shell.html)
set_target_properties(game PROPERTIES SUFFIX .html)
else()
target_compile_features(game PRIVATE cxx_std_11)
endif()
target_link_libraries(game PRIVATE raylib_static)