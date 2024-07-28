#pragma once

#include <routing_game.hpp>
#include <iostream>

class MockCardInterface : public rg::CardCommInterface {
private:
    rg::CardLogicalId id;
    std::vector<rg::PacketVisit> visits;
    std::bitset<32> metadata;

public:
    MockCardInterface(rg::CardSeqNum seq_id) {
        id.seq = seq_id;
    }

    rg::CardLogicalId get_id() override {
        return id;
    }

    rg::CardSeqNum get_seq() override {
        return id.seq;
    }

    int visit_count() override {
        return visits.size();
    }

    rg::PacketVisit get_visit(int idx) override {
        if (idx < 0)
            idx = visits.size() + idx;
        assert(idx >= 0 && idx < visits.size());
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
