#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "topology_samples.hpp"

TEST_CASE("Network: Check Square topology") {
    auto json = rg::jsonSquareTopology();

    auto setup = rg::io::round_setup_from_json('A', json);

    CHECK(setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'A'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'B'));
    CHECK(!setup.network().are_neighbors('A', 'C'));
    CHECK(!setup.network().are_neighbors('C', 'A'));

    CHECK(setup.network().are_neighbors('A', 'D'));
    CHECK(setup.network().are_neighbors('D', 'A'));    
}