#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("TCP packet: not implemented") { // TODO: TCP packet type
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 6;
    CHECK(setup.packet_info(card.get_seq()).type == rg::PacketType::TCP);
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Invalid);
}
