#include <iostream>
#include "routing_game.hpp"

using namespace rg;

void rg::apply_topology_event(rg::Network& network, const rg::TopologyEvent& event) {
    switch (event.type) {
        case TopologyEventType::LinkUp: {
            assert(event.link.has_value());
            auto [from, to] = *event.link;
            network.add_connection(from, to);
        } break;

        case TopologyEventType::LinkDown: {
            assert(event.link.has_value());
            auto [from, to] = *event.link;
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
        return {
            .action = {.result = PacketVisitResult::Finished, .log = "Points already awarded"}
        };
    }

    if (card.visit_count() > 0 && card.get_visit(-1).where == me) {
        return {
            .action = {
                .result = PacketVisitResult::Continue,
                .instructions = info,
                .log = "Multibeep"
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

        return result_handle_internal {
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
            .instructions = info,
            .log = "Packet in transit"
        },
        .log = log
    };
}

/**
 * Analogy to standard packet, but the points are awarded based on the time the packet took to get delivered.
 * The reward is packet.basePoints * (<minutes before deadline>+1)
 */
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

        result.action.log = "Awarded points; mult=" + std::to_string(multiplier) + ", time=" + std::to_string(time);
    }

    return result;
}

/**
 * This packet awards packet.pointsPerHop points for every hop it takes. Going back counts as well.
 */
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
                .log = "Entering game",
            },
            .log = log
        };
    }

    if (card.get_visit(-1).where == me) {
        return {
            .action = {
                .result = PacketVisitResult::Continue,
                .instructions = std::to_string(card.visit_count() - 1),
                .log = "Multibeep",
            }
        };
    }

    auto packet = setup.packet_info(card.get_seq());

    rg::PacketVisit log = {
        .where = me,
        .time = setup.time(),
        .points = packet.pointsPerHop,
    };

    std::string logText = "Hop " + card.get_visit(-1).where;
    logText += "->" + me;

    return {
        .action = {
            .result = PacketVisitResult::Continue,
            .instructions = std::to_string(card.visit_count()),
            .points = packet.pointsPerHop,
            .log = logText,
        },
        .log = log
    };
}

/**
 * This packet awards packet.points points (default is 10, but 50 is standard)
 * after it has visited every router in the network.
 * The visited routers are stored in metadata[<routerName>-'A'].
 */
result_handle_internal handle_visitall_packet(const rg::RoundSetup& setup, rg::CardCommInterface& card) {
    auto me = setup.who_am_i();

    int routerCount = setup.network().routers().size();

    auto metadata = card.get_metadata();

    if (metadata.count() >= routerCount) {
        // card is already completed
        return {
            .action = {.result = PacketVisitResult::Finished}
        };
    }

    metadata.set(me - 'A', true);
    card.set_metadata(metadata);

    if (metadata.count() < routerCount) {
        // this is not the last router
        if (card.visit_count() > 0 && card.get_visit(-1).where == me) {
            // don't log repeated beep
            return {
                .action = {
                    .result = PacketVisitResult::Continue,
                    .instructions = std::to_string(metadata.count()),
                    .log =  "Bitmap = " + metadata.to_string() + "; Multibeep at " + me,
                }
            };
        } else {
            // log visit
            rg::PacketVisit log = {
                .where = me,
                .time = setup.time(),
            };
            return {
                .action = {
                    .result = PacketVisitResult::Continue,
                    .instructions = std::to_string(metadata.count()),
                    .log = "Bitmap = " + metadata.to_string() + "; Revisiting " + me
                },
                .log = log
            };

        }
    }

    // this is the finishing beep => award points!
    int points = setup.packet_info(card.get_seq()).points;

    rg::PacketVisit log = {
        .where = me,
        .time = setup.time(),
        .points = points
    };
    return {
        .action = {
            .result = PacketVisitResult::Finished,
            .instructions = std::to_string(metadata.count()),
            .points = points,
            .log =  "Bitmap = " + metadata.to_string(),
        },
        .log = log
    };
}

rg::UiAction rg::handle_packet_visit(const rg::RoundSetup& setup, rg::CardCommInterface& card) {

    auto packet = setup.packet_info(card.get_seq());
    auto me = setup.who_am_i();

    if (card.visit_count() == 0) {
        if (packet.source != me) {
            return {
                .result = PacketVisitResult::Invalid,
                .log = "Invalid packet entry point"
            };
        }
    } else {
        auto previous = card.get_visit(-1).where;
        if (! setup.network().are_neighbors(previous, me)) {
            std::string logText = "Invalid hop: " + previous;
            logText += "->" + me;
            return {
                .result = PacketVisitResult::Invalid,
                .log = logText,
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
        case PacketType::VisitAll:
            result = handle_visitall_packet(setup, card);
            break;        default:
            return {
                .result = PacketVisitResult::Invalid,
                .log = "Unrecognized packet type: " + std::to_string(static_cast<int>(packet.type)),
            };
    }

    if (result.log.has_value()) {
        card.mark_visit(result.log.value());
    }

    return result.action;
}
