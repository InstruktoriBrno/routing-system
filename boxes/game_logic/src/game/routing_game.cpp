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

rg::UiAction rg::handle_packet_visit(const rg::RoundSetup& setup, rg::CardCommInterface& card) {
    // IMPLEMENT THIS FUNCTION

    card.mark_visit({
        .where = setup.who_am_i(),
        .timestamp = setup.time(),
        .any_points_awarded = false,
        .points = 0
    });

    return {
        .valid = true
    };
}
