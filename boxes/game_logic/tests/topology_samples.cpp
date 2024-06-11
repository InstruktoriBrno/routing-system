 #include "topology_samples.hpp"
 
 using namespace rg;

nlohmann::json rg::jsonSquareTopology() {
    return nlohmann::json::parse(R"(
        {
            "round_id": 42,
            "duration": 1200,
            "routers": {
                "A": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                },
                "B": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                },
                "C": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                },
                "D": {
                    "mac": "xx:xx:xx:xx:xx:xx"
                }
            },
            "links": [ "AB", "BC", "CD", "DA" ],
            "packets": {    
                "1": {
                    "type": "standard",
                    "source": "A",
                    "destination": "C",
                    "points": 7
                }
            },
            "events": [
            ]
        }
    )");
}