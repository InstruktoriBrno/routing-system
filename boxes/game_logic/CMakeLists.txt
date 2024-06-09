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
    GIT_TAG d345fcf
)

FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_MakeAvailable(doctest json)

# Add test with CTest
enable_testing()

add_subdirectory(src/game)
add_subdirectory(src/full_io)
add_subdirectory(src/bin)
add_subdirectory(tests)


