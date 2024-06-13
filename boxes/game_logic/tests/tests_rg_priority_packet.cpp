#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("Priority packet: deliver immediatelly") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    int pointsExpected = 20;

    MockCardInterface card;
    card.id.seq = 2;
    card.visits.push_back({.where = 'A', .time = 1});
    card.visits.push_back({.where = 'B', .time = 2});

    setup.advance_time_to(3);

    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(action.points == pointsExpected);

    CHECK(card.visit_count() == 3);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'C');
    CHECK(visit.points == pointsExpected);

    CHECK(card.get_metadata()[0]);
}

TEST_CASE("Priority packet: deliver on deadline") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    int pointsExpected = 4;

    MockCardInterface card;
    card.id.seq = 2;
    card.visits.push_back({.where = 'A', .time = 8});
    card.visits.push_back({.where = 'B', .time = 2});

    setup.advance_time_to(308);

    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(action.points == pointsExpected);

    CHECK(card.visit_count() == 3);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'C');
    CHECK(visit.points == pointsExpected);

    CHECK(card.get_metadata()[0]);
}

TEST_CASE("Priority packet: deliver after deadline") {
    auto setup = rg::io::round_setup_from_json('C', rg::jsonSquareTopology());
    int pointsExpected = 0;

    MockCardInterface card;
    card.id.seq = 2;
    card.visits.push_back({.where = 'A', .time = 8});
    card.visits.push_back({.where = 'B', .time = 2});

    setup.advance_time_to(309);

    auto action = rg::handle_packet_visit(setup, card);

    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(action.points == pointsExpected);

    CHECK(card.visit_count() == 3);
    auto visit = card.get_visit(-1);
    CHECK(visit.where == 'C');
    CHECK(visit.points == pointsExpected);

    CHECK(card.get_metadata()[0]);
}