#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>

TEST_CASE("Basic game setup") {
    auto json = nlohmann::json::parse(R"(
        {
            "routers": {
                "0": {},
                "1": {},
                "2": {}
            },
            "links": [
                [0, 1],
                [1, 2],
                [2, 0]
            ],
            "packets": {
                "0": {
                    "type": "standard",
                    "source": 0,
                    "destination": 2
                }
            },
            "events": [
                {
                    "type": "linkdown",
                    "timestamp": 3,
                    "edge": [0, 1]
                },
                {
                    "type": "linkup",
                    "timestamp": 4,
                    "edge": [0, 1]
                }
            ]
        }
    )");

    auto setup = rg::io::round_setup_from_json(0, json);

    CHECK(setup.network().are_neighbors(0, 1));
    CHECK(setup.network().are_neighbors(1, 2));
    CHECK(setup.network().are_neighbors(2, 0));

    setup.advance_time_to(1);
    CHECK(setup.network().are_neighbors(0, 1));
    CHECK(setup.network().are_neighbors(1, 2));
    CHECK(setup.network().are_neighbors(2, 0));

    setup.advance_time_to(2);
    CHECK(setup.network().are_neighbors(0, 1));
    CHECK(setup.network().are_neighbors(1, 2));
    CHECK(setup.network().are_neighbors(2, 0));

    setup.advance_time_to(3);
    CHECK(!setup.network().are_neighbors(0, 1));
    CHECK(setup.network().are_neighbors(1, 2));
    CHECK(setup.network().are_neighbors(2, 0));

    setup.advance_time_to(4);
    CHECK(setup.network().are_neighbors(0, 1));
    CHECK(setup.network().are_neighbors(1, 2));
    CHECK(setup.network().are_neighbors(2, 0));


}
