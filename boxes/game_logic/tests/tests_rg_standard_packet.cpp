#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("Standard packet: enter game") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 1;
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    
    CHECK(card.visit_count() == 1);

    auto visit = card.get_visit(0);
    CHECK(visit.where == 'A');
    CHECK(visit.time == 12);
    CHECK(visit.points == 0);
}

TEST_CASE("Standard packet: retry initial router") {
    auto setup = rg::io::round_setup_from_json('A', rg::jsonSquareTopology());

    MockCardInterface card;
    card.id.seq = 1;
    card.visits.push_back({
        .where = 'A'
    });
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    CHECK(!action.points.has_value());
    CHECK(card.visit_count() == 1);
    CHECK(!card.get_metadata()[0]);
}

TEST_CASE("Standard packet: first hop") {
    auto setup = rg::io::round_setup_from_json('B', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 1;
    card.visits.push_back({
        .where = 'A'
    });
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    CHECK(!action.points.has_value());

    CHECK(card.visit_count() == 2);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'B');
    CHECK(visit.time == 12);
    CHECK(visit.points == 0);

    CHECK(!card.get_metadata()[0]);
}

TEST_CASE("Standard packet: reach destination") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 1;
    card.visits.push_back({.where = 'A'});
    card.visits.push_back({.where = 'B'});
    
    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(action.points == 10);

    CHECK(card.visit_count() == 3);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'C');
    CHECK(visit.time == 12);
    CHECK(visit.points == 10);

    CHECK(card.get_metadata()[0]);
}

TEST_CASE("Standard packet: revisit destination") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    setup.advance_time_to(12);

    MockCardInterface card;
    card.id.seq = 1;
    card.visits.push_back({.where = 'A'});
    card.visits.push_back({.where = 'B'});
    card.set_metadata(0b1);

    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(!action.points.has_value());

    CHECK(card.visit_count() == 2);

    CHECK(card.get_metadata()[0]);
}