#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>

TEST_CASE("Basic game setup") {
    auto json = nlohmann::json::parse(R"(
        {
            "round_id": 42,
            "routers": {
                "A": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                },
                "B": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                },
                "C": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                }
            },
            "links": [ "AB", "BC", "CA" ],
            "packets": {
                "0": {
                    "type": "standard",
                    "source": "A",
                    "destination": "C"
                }
            },
            "events": [
                {
                    "type": "linkdown",
                    "time": 3,
                    "edge": "AB"
                },
                {
                    "type": "linkup",
                    "time": 4,
                    "edge": "AB"
                }
            ]
        }
    )");

    auto setup = rg::io::round_setup_from_json(0, json);

    CHECK(setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'A'));

    setup.advance_time_to(1);
    CHECK(setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'A'));

    setup.advance_time_to(2);
    CHECK(setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'A'));

    setup.advance_time_to(3);
    CHECK(!setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'A'));

    setup.advance_time_to(4);
    CHECK(setup.network().are_neighbors('A', 'B'));
    CHECK(setup.network().are_neighbors('B', 'C'));
    CHECK(setup.network().are_neighbors('C', 'A'));


}
