cmake_minimum_required(VERSION 3.14)
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_executable(routing_game_tests
    topology_samples.cpp
    tests_main.cpp
    tests_rg_network.cpp
    tests_rg_game_setup.cpp
    tests_rg_standard_packet.cpp
)
target_link_libraries(routing_game_tests PRIVATE
    rg::game
    rg::io
    doctest::doctest)

doctest_discover_tests(routing_game_tests)
