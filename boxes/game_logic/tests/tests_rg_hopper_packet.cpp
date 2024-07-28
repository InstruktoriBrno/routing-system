#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("Hopper packet: enter game") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    setup.advance_time_to(1);

    MockCardInterface card(3);
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "0");
    
    CHECK(card.visit_count() == 1);

    auto visit = card.get_visit(0);
    CHECK(visit.where == 'A');
    CHECK(visit.time == 1);
    CHECK(visit.points == 0);
}

TEST_CASE("Hopper packet: retry initial router") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());

    MockCardInterface card(3);
    card.mark_visit({.where = 'A'});
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "0");
    CHECK(!action.points.has_value());
    CHECK(card.visit_count() == 1);
}

TEST_CASE("Hopper packet: first hop") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card(3);
    card.mark_visit({.where = 'A'});
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "1");
    CHECK(action.points == 1);

    CHECK(card.visit_count() == 2);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'B');
    CHECK(visit.time == 12);
    CHECK(visit.points == 1);
}

TEST_CASE("Hopper packet: repeat router") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card(3);
    card.mark_visit({.where = 'A'});
    card.mark_visit({.where = 'B'});
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "1");
    CHECK(!action.points.has_value());

    CHECK(card.visit_count() == 2);
}

TEST_CASE("Hopper packet: multihop test") {
    auto setupA = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    auto setupB = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    auto setupC = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    auto setupD = rg::io::round_setup_from_json('D', rg::jsonSquareTopology());

    MockCardInterface card(3);

    rg::handle_packet_visit(setupB, card); // invalid
    rg::handle_packet_visit(setupA, card); // start
    rg::handle_packet_visit(setupB, card); // 1
    rg::handle_packet_visit(setupB, card); // repeat
    rg::handle_packet_visit(setupC, card); // 2
    rg::handle_packet_visit(setupA, card); // invalid
    rg::handle_packet_visit(setupC, card); // repeat
    rg::handle_packet_visit(setupB, card); // 3
    rg::handle_packet_visit(setupA, card); // 4
    rg::handle_packet_visit(setupA, card); // repeat

    auto action = rg::handle_packet_visit(setupD, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "5");
    CHECK(action.points == 1);

    CHECK(card.visit_count() == 6);
}