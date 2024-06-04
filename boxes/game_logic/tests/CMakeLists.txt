cmake_minimum_required(VERSION 3.14)

add_executable(routing_game_tests
    tests_main.cpp
    tests_rg_network.cpp
    tests_rg_game_setup.cpp
)
target_link_libraries(routing_game_tests PRIVATE
    rg::game
    rg::io
    doctest::doctest)
