#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("VisitAll packet: enter game") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());

    MockCardInterface card(4);
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "1");
    CHECK(!action.points.has_value());
    
    CHECK(card.visit_count() == 1);

    auto visit = card.get_visit(0);
    CHECK(visit.where == 'A');
    CHECK(visit.points == 0);

    CHECK(card.get_metadata() == 0x1);
}

TEST_CASE("VisitAll packet: visit last router ") {
    auto setup = rg::io::round_setup_from_json('D', rg::jsonSquareTopology());

    MockCardInterface card(4);
    card.mark_visit({.where = 'A'});
    card.mark_visit({.where = 'B'});
    card.mark_visit({.where = 'C'});
    card.set_metadata(0x7);

    auto action = rg::handle_packet_visit(setup, card);

    std::cout << "Resulting action = " << action.instructions.value() << ", " << (int) action.result << ", " << action.points.value() << "\n\n\n\n";
    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(action.instructions == "4");
    CHECK(action.points == 50);
    
    CHECK(card.visit_count() == 4);

    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'D');
    CHECK(visit.points == 50);

    CHECK(card.get_metadata() == 0xf);
}

TEST_CASE("VisitAll packet: visit second router") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());

    MockCardInterface card(4);
    card.mark_visit({.where = 'A'});
    card.set_metadata(0x1);
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "2");
    CHECK(!action.points.has_value());
    CHECK(card.visit_count() == 2);
    CHECK(card.get_metadata() == 0x3);
}

TEST_CASE("VisitAll packet: revisit first router") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());

    MockCardInterface card(4);
    card.mark_visit({.where = 'A'});
    card.mark_visit({.where = 'B'});
    card.set_metadata(0x3);
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "2");
    CHECK(!action.points.has_value());
    CHECK(card.visit_count() == 3);
    CHECK(card.get_metadata() == 0x3);
}


TEST_CASE("VisitAll packet: multistep test") {
    auto setupA = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    auto setupB = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    auto setupC = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    auto setupD = rg::io::round_setup_from_json('D', rg::jsonSquareTopology());

    MockCardInterface card(4);

    rg::handle_packet_visit(setupB, card); // invalid
    rg::handle_packet_visit(setupA, card); // 1
    rg::handle_packet_visit(setupB, card); // 2
    rg::handle_packet_visit(setupB, card); // repeat
    rg::handle_packet_visit(setupC, card); // 3
    rg::handle_packet_visit(setupA, card); // invalid
    rg::handle_packet_visit(setupC, card); // repeat
    rg::handle_packet_visit(setupB, card); // 3
    rg::handle_packet_visit(setupA, card); // 3
    rg::handle_packet_visit(setupA, card); // repeat

    auto action = rg::handle_packet_visit(setupD, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(action.instructions == "4");
    CHECK(action.points == 50);
    CHECK(card.visit_count() == 6);
    CHECK(card.get_metadata() == 0xf);
}