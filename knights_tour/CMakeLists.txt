cmake_minimum_required(VERSION 3.28)
project(knight_tour LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(main ${SOURCE_FILES})
