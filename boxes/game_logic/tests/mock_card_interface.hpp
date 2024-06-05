#pragma once

#include <routing_game.hpp>

class MockCardInterface : public rg::CardCommInterface {
public:
    rg::CardLogicalId id;
    std::vector<rg::PacketVisit> visits;
    std::bitset<32> metadata;

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

    std::bitset<32> get_metadata() override {
        return metadata;
    }

    void set_metadata(std::bitset<32> new_metadata) override {
        metadata = new_metadata;
    }
};
