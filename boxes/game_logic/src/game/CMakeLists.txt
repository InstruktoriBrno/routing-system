cmake_minimum_required(VERSION 3.14)

# Add the main library
add_library(routing_game
    routing_game.cpp)
target_include_directories(routing_game PUBLIC .)
add_library(rg::game ALIAS routing_game)

