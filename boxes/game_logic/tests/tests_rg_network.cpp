#include <doctest/doctest.h>
#include <routing_game.hpp>

TEST_CASE("Basic network") {
    rg::Network network;

    for (int i = 0; i < 3; i++) {
        network.add_router(i);
    }

    network.add_connection(0, 1);
    CHECK(network.are_neighbors(0, 1));
    CHECK(network.are_neighbors(1, 0));

    network.add_connection(1, 2);
    CHECK(network.are_neighbors(1, 2));
    CHECK(network.are_neighbors(2, 1));
}
