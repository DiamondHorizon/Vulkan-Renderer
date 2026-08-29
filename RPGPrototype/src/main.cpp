#include "app.hpp"

// std = standard
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        vulkan::App app{};
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Notes

// Product ID - 8b3a12badbd0423c8f93ade7af0c8972
// Application ID - fghi45670nlnDv3UjRqBRRYbmZuqcUsX
// Sandbox ID - cc09e301ccbf483fb7c352b90b2f9565
// Deployment ID - 7b31619f37ce4c90b870a7b8b7d32b26
// Client ID - xyza7891mU8nNGXp6zxi22pZQrtuQQIa
// Client Secret - cakA54YBgCPYJZely4WY/85XogFvMvnSCtSniggp8cE

// Account Ids

// DiamondHorizon88 - fff6c35e6ad24cadaea3138211d02a4e
// LegoDog2014 - 374895bf061a4f849cb9f06ee7afe37a