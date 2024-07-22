#include <doctest/doctest.h>
#include <routing_game.hpp>
#include <io_routing_game.hpp>
#include "mock_card_interface.hpp"
#include "topology_samples.hpp"

TEST_CASE("TCP packet: quick passthrough") {
    // Fastest delivery possible, both there and back again.
    // Repetitively beeped at router B, which does not count as extra hops.
    MockCardInterface card;
    card.id.seq = 6;

    auto roundSpec = rg::jsonSquareTopology();
    rg::UiAction action;

    auto routerA = rg::io::round_setup_from_json('A', roundSpec);
    auto routerB = rg::io::round_setup_from_json('B', roundSpec);
    auto routerC = rg::io::round_setup_from_json('C', roundSpec);
    auto routerD = rg::io::round_setup_from_json('D', roundSpec);

    CHECK(routerA.packet_info(card.get_seq()).type == rg::PacketType::TCP);

    routerA.advance_time_to(70);
    action = rg::handle_packet_visit(routerA, card);
    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    CHECK(card.visit_count() == 1);

    routerB.advance_time_to(75);
    action = rg::handle_packet_visit(routerB, card);
    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    CHECK(card.visit_count() == 2);

    routerB.advance_time_to(80);
    action = rg::handle_packet_visit(routerB, card);
    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "C");
    CHECK(card.visit_count() == 2); // multibeep ignored

    routerC.advance_time_to(129);
    action = rg::handle_packet_visit(routerC, card);
    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "A"); // now, deliver it back to the source
    CHECK(card.visit_count() == 3);

    routerA.advance_time_to(135);
    action = rg::handle_packet_visit(routerA, card);
    CHECK(action.result == rg::PacketVisitResult::Invalid);
    CHECK(action.instructions == "A");
    CHECK(card.visit_count() == 3); // invalid beep ignored

    routerD.advance_time_to(140);
    action = rg::handle_packet_visit(routerD, card);
    CHECK(action.result == rg::PacketVisitResult::Continue);
    CHECK(action.instructions == "A");
    CHECK(card.visit_count() == 4);

    routerA.advance_time_to(188);
    auto action4 = rg::handle_packet_visit(routerA, card);
    CHECK(action4.result == rg::PacketVisitResult::Finished);
    CHECK(!action4.instructions.has_value());
    CHECK(card.visit_count() == 5);

    routerB.advance_time_to(160);
    action = rg::handle_packet_visit(routerB, card);
    CHECK(action.result == rg::PacketVisitResult::Finished);
    CHECK(!action.instructions.has_value());
    CHECK(card.visit_count() == 5);


    // now check the visit log as recorded on the card

    CHECK(card.get_visit(0).where == 'A');
    CHECK(card.get_visit(0).time == 70);
    CHECK(card.get_visit(0).points == 0);

    CHECK(card.get_visit(1).where == 'B');
    CHECK(card.get_visit(1).time == 75);
    CHECK(card.get_visit(1).points == 0);

    // repeated beep at router B not marked as a visit

    CHECK(card.get_visit(2).where == 'C');
    CHECK(card.get_visit(2).time == 129);
    CHECK(card.get_visit(2).points == 6);

    // invalid beep at router A not marked as a visit

    CHECK(card.get_visit(3).where == 'D');
    CHECK(card.get_visit(3).time == 140);
    CHECK(card.get_visit(3).points == 0);

    CHECK(card.get_visit(4).where == 'A');
    CHECK(card.get_visit(4).time == 188);
    CHECK(card.get_visit(4).points == 6);
}

TEST_CASE("TCP packet: no source visit") {
    // The packet is not marked at the source, and delivered directly, which does not
    // count.
    CHECK(true); // TODO
}

TEST_CASE("TCP packet: late pickup") {
    // The packet is released at time 0, but only marked at source at time 35 - at which
    // point it is considered as already being delivered for 5 seconds.
    CHECK(true); // TODO
}

TEST_CASE("TCP packet: back-and-forth routing") {
    // The packet gets pinged between A and B multiple times before it reaches its
    // destination C, already after too many hops to get any points for that delivery.
    // Gets points on the way back, though.
    // Also, visits following the final delivery to source again are considered as
    // invalid.

}
