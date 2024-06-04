cmake_minimum_required(VERSION 3.14)
project(routing_game LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG v2.4.11
)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

    FetchContent_MakeAvailable(doctest json)

add_subdirectory(src/game)
add_subdirectory(src/full_io)
add_subdirectory(src/bin)
add_subdirectory(tests)

# Add test with CTest
enable_testing()
add_test(NAME routing_game_tests COMMAND routing_game_tests)

