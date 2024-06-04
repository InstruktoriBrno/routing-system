#pragma once

#include <routing_game.hpp>

class MockCardInterface : public rg::CardCommInterface {
public:
    rg::CardLogicalId id;
    std::vector<rg::PacketVisit> visits;

    rg::CardLogicalId get_id() override {
        return id;
    }

    int visit_count() override {
        return visits.size();
    }

    rg::PacketVisit get_visit(int idx) override {
        return visits[idx];
    }

    void mark_visit(rg::PacketVisit visit) override {
        visits.push_back(visit);
    }
};
