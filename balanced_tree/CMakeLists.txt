cmake_minimum_required(VERSION 3.28)
project(balanced_tree LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(main ${SOURCE_FILES})
target_link_libraries(main PRIVATE SFML::Graphics)
