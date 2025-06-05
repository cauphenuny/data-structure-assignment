cmake_minimum_required(VERSION 3.28)
project(balanced_tree LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

if(MSVC)
  add_compile_options(/W4)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(/O2)
  endif()
else()
  add_compile_options(-Wall -Wextra)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
  endif()
endif()

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

file(GLOB BALANCED_TREE_SOURCES "balanced_tree/src/*.cpp" "balanced_tree/src/test/*.cpp")
add_executable(balanced_tree ${BALANCED_TREE_SOURCES})
target_include_directories(balanced_tree PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/balanced_tree/include"
)
target_include_directories(balanced_tree PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
target_link_libraries(balanced_tree PRIVATE SFML::Graphics)

file(GLOB KNIGHTS_TOUR_SOURCES "knights_tour/src/*.cpp")
add_executable(knights_tour ${KNIGHTS_TOUR_SOURCES})
target_include_directories(knights_tour PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/knights_tour/include"
)
target_include_directories(knights_tour PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
target_link_libraries(knights_tour PRIVATE SFML::Graphics)

enable_testing()
add_test(NAME BalancedTree COMMAND balanced_tree test)
