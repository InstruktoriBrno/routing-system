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

TEST_CASE("Admin packet: check displayed info") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    setup.advance_time_to(-1);

    MockCardInterface card;
    card.id.seq = 0;    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "B");

    setup.advance_time_to(100);
    action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "B");
}

TEST_CASE("Locator packet: pre-game success") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    setup.advance_time_to(-1);

    MockCardInterface card;
    card.id.seq = 5;    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(!action.instructions.has_value());
}

TEST_CASE("Locator packet: pre-game fail") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    setup.advance_time_to(-1);

    MockCardInterface card;
    card.id.seq = 5;    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Invalid);
    CHECK(!action.instructions.has_value());
}

TEST_CASE("Locator packet: in-game success") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    setup.advance_time_to(1);

    MockCardInterface card;
    card.id.seq = 5;    
    auto action = rg::handle_packet_visit(setup, card);
    std::cerr << action.log << "\n\n\n";

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
}

TEST_CASE("Locator packet: in-game fail") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    setup.advance_time_to(1);

    MockCardInterface card;
    card.id.seq = 5;    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Invalid);
    CHECK(!action.instructions.has_value());
}

TEST_CASE("Arbitrary packet: after game ended") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());

    std::cerr << "WTF??? " << setup.time() << " " << setup.duration() << "\n";
    setup.advance_time_to(1500);

    MockCardInterface card;
    card.id.seq = 1;    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
}
