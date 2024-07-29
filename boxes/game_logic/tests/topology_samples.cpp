 #include "topology_samples.hpp"
 
 using namespace rg;

nlohmann::json rg::jsonSquareTopology() {
    return nlohmann::json::parse(R"(
        {
            "roundId": 42,
            "roundName": "Test: Round 1",
            "duration": 1200,
            "routers": {
                "A": {
                    "mac": ["xx:xx:xx:xx:xx:xx"]
                },
                "B": {
                    "mac": ["xx:xx:xx:xx:xx:xx"]
                },
                "C": {
                    "mac": ["xx:xx:xx:xx:xx:xx"]
                },
                "D": {
                    "mac": ["xx:xx:xx:xx:xx:xx"]
                }
            },
            "links": [ "AB", "BC", "CD", "DA" ],
            "packets": {
                "0": {
                    "type": "admin",
                    "releaseTime": 0,
                    "source": "C"
                },
                "1": {
                    "type": "standard",
                    "releaseTime": 0,
                    "source": "A",
                    "destination": "C"
                },
                "2": {
                    "type": "priority",
                    "releaseTime": 0,
                    "source": "A",
                    "destination": "C"
                },
                "3": {
                    "type": "hopper",
                    "releaseTime": 0,
                    "source": "A",
                    "pointsPerHop": 3
                },
                "4": {
                    "type": "visitall",
                    "releaseTime": 0,
                    "source": "A",
                    "points": 50
                },
                "5": {
                    "type": "locator",
                    "releaseTime": 0,
                    "source": "C"
                },
                "6": {
                    "type": "tcp",
                    "releaseTime": 65,
                    "source": "A",
                    "destination": "C",
                    "points": 10
                },
                "7": {
                    "type": "chat",
                    "releaseTime": 65,
                    "source": "A",
                    "destination": "C",
                    "points": 10,
                    "roundTripCount": 3,
                    "messages": ["SYN", "SYN/ACK", "ACK", "Wazzup!", "Wazzuup!", "Wazzuuup!"]
                }
            },
            "events": [
            ]
        }
    )");
}