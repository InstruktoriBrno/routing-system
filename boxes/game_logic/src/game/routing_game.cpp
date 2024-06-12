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

struct result_handle_internal {
    rg::UiAction action;
    std::optional<rg::PacketVisit> log;
};

/**
 * A standard packet starts at packet.source. When it reaches packet.destination,
 * then packet.points are awarded to the team.
 * After the points are awarded, a PacketVisitResult::Finished with no points
 * is returned by all routers.
 * If the points were already awarded, then card_getMetadata()[0] == true is set 
 * to avoid reading the whole trace.
 */
result_handle_internal handle_standard_packet(const rg::RoundSetup& setup, rg::CardCommInterface& card) {
    rg::PacketInfo packet = setup.packet_info(card.get_seq());
    assert(packet.destination.has_value());
    auto destination = packet.destination.value();

    std::string info;
    info += destination;

    auto me = setup.who_am_i();

    if (card.get_metadata()[0]) {
        rg::UiAction action = {.result = PacketVisitResult::Finished,};
        std::optional<rg::PacketVisit> log = std::nullopt;
        return {
            action = {.result = PacketVisitResult::Finished,}
        };
    }

    if (card.visit_count() > 0 && card.get_visit(-1).where == me) {
        return {
            .action = {
                .result = PacketVisitResult::Continue,
                .instructions = info
            }
        };
    }

    if (me == destination) {
        auto metadata = card.get_metadata();
        metadata.set(0, 1);
        card.set_metadata(metadata);
        
        assert(card.get_metadata()[0]);
        int points = packet.points;
        assert(points == 10);

        rg::PacketVisit log = {
            .where = me,
            .time = setup.time(),
            .points = points
        };

        return {
            .action = {
                .result = PacketVisitResult::Finished,
                .points = points
            },
            .log = log
        };
    }

    rg::PacketVisit log = {
        .where = me,
        .time = setup.time(),
    };

    return {
        .action = {
            .result = PacketVisitResult::Continue,
            .instructions = info
        },
        .log = log
    };
}

result_handle_internal handle_priority_packet(const rg::RoundSetup& setup, rg::CardCommInterface& card) {

    result_handle_internal result = handle_standard_packet(setup, card);

    if (result.log.has_value() && result.log.value().points > 0) {
        int time = setup.time() - card.get_visit(0).time;
        int pointsBase = setup.packet_info(card.get_id().seq).pointsPerMinuteLeft;
        int minutesToDeliver = setup.packet_info(card.get_id().seq).minutesToDeliver;

        int multiplier = ((minutesToDeliver+1)*60 - time) / 60;
        int points = pointsBase * multiplier;
        points = points >= 0 ? points : 0;

        result.log.value().points = points;
        result.action.points = points;
    }

    return result;
}

result_handle_internal handle_hopper_packet(const rg::RoundSetup& setup, rg::CardCommInterface& card) {
    char me = setup.who_am_i();

    if (card.visit_count() == 0) {
        rg::PacketVisit log = {
            .where = me,
            .time = setup.time(),
        };

        return {
            .action = {
                .result = PacketVisitResult::Continue,
                .instructions = std::to_string(card.visit_count()),
            },
            .log = log
        };
    }

    if (card.get_visit(-1).where == me) {
        return {
            .action = {
                .result = PacketVisitResult::Continue,
                .instructions = std::to_string(card.visit_count() - 1),
            }
        };
    }

    auto packet = setup.packet_info(card.get_seq());

    rg::PacketVisit log = {
        .where = me,
        .time = setup.time(),
        .points = packet.pointsPerHop,
    };

    return {
        .action = {
            .result = PacketVisitResult::Continue,
            .instructions = std::to_string(card.visit_count()),
            .points = packet.pointsPerHop,
        },
        .log = log
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

    result_handle_internal result;

    switch (packet.type) {

        case PacketType::Standard: 
            result = handle_standard_packet(setup, card);
            break;
        case PacketType::Priority:
            result = handle_priority_packet(setup, card);
            break;
        case PacketType::Hopper:
            result = handle_hopper_packet(setup, card);
            break;
        default: 
            return {
                .result = PacketVisitResult::Invalid
            };
    }

    if (result.log.has_value()) {
        card.mark_visit(result.log.value());
    }

    return result.action;
}
