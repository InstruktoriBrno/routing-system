#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("Any packet: Wrong starting point") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());

    MockCardInterface card;
    card.id.seq = 1;
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Invalid);
    CHECK(card.visit_count() == 0);
    CHECK(card.get_metadata() == 0b00000000000000000000000000000000);
}

TEST_CASE("Any packet: invalid hop") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 1;
    card.visits.push_back({
        .where = 'A'
    });
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Invalid);
    CHECK(card.visit_count() == 1);
    CHECK(card.get_metadata() == 0b00000000000000000000000000000000);
}
