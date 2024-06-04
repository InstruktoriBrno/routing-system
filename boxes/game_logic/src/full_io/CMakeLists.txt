cmake_minimum_required(VERSION 3.14)

# Add the main library
add_library(routing_full_io STATIC
    io_routing_game.cpp)
target_include_directories(routing_full_io PUBLIC .)
target_link_libraries(routing_full_io PUBLIC routing_game nlohmann_json::nlohmann_json)
add_library(rg::io ALIAS routing_full_io)

