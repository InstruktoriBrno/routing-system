#include <io_routing_game.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    try {
        auto json = nlohmann::json::parse(std::cin);
        auto network = rg::io::from_json<rg::Network>(json);
        std::cout << rg::io::to_dot(network) << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
