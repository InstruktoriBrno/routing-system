#include <iostream>
#include "routing_game.hpp"

using namespace rg;

void rg::apply_topology_event(rg::Network& network, const rg::TopologyEvent& event) {
    switch (event.type) {
        case TopologyEventType::LinkUp: {
            assert(event.edge.has_value());
            auto [from, to] = *event.edge;
            network.add_connection(from, to);
        } break;

        case TopologyEventType::LinkDown: {
            assert(event.edge.has_value());
            auto [from, to] = *event.edge;
            network.remove_connection(from, to);
        }  break;
    }
}

/**
 * A standard packet starts at packet.source. When it reaches packet.destination,
 * then packet.points are awarded to the team.
 * After the points are awarded, a PacketVisitResult::Finished with no points
 * is returned by all routers.
 * If the points were already awarded, then card_getMetadata()[0] == true is set 
 * to avoid reading the whole trace.
 */
rg::UiAction handle_standard_packet(const rg::RoundSetup& setup, rg::CardCommInterface& card) {
    rg::PacketInfo packet = setup.packet_info(card.get_seq());
    assert(packet.destination.has_value());
    auto destination = packet.destination.value();

    std::string info;
    info += destination;

    auto me = setup.who_am_i();

    if (card.get_metadata()[0]) {
        return {
            .result = PacketVisitResult::Finished,
        };
    }

    if (card.visit_count() > 0 && card.get_visit(-1).where == me) {
        return {
            .result = PacketVisitResult::Continue,
            .instructions = info
        };
    }

    if (me == destination) {
        auto metadata = card.get_metadata();
        metadata.set(0, 1);
        card.set_metadata(metadata);
        
        assert(card.get_metadata()[0]);
        int points = packet.points.has_value() ? packet.points.value() : 10;

        card.mark_visit({
            .where = me,
            .time = setup.time(),
            .points = points
        });

        return {
            .result = PacketVisitResult::Finished,
            .points = points
        };
    }

    card.mark_visit({
        .where = me,
        .time = setup.time(),
    });

    return {
        .result = PacketVisitResult::Continue,
        .instructions = info
    };
}

rg::UiAction rg::handle_packet_visit(const rg::RoundSetup& setup, rg::CardCommInterface& card) {

    auto packet = setup.packet_info(card.get_seq());
    auto me = setup.who_am_i();

    std::cout << "Handling packet\n";

    if (card.visit_count() == 0) {
        if (packet.source != me) {
            std::cout << "Invalid start location " << me << ": expected " << packet.source << "\n";
            return {
                .result = PacketVisitResult::Invalid
            };
        }
    } else {
        auto previous = card.get_visit(-1).where;
        if (! setup.network().are_neighbors(previous, me)) {
            std::cout << "Invalid hop " << previous << "->" << me << "\n";
            return {
                .result = PacketVisitResult::Invalid
            };
        }
    }

    switch (packet.type) {

        case PacketType::Standard: 
            std::cout << "  standard packet\n";
            return handle_standard_packet(setup, card);

        default: 
            std::cout << "Unknown packet\n";
            return {
                .result = PacketVisitResult::Invalid
            };
    }
}
