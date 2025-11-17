#include <iostream>
#include <stdexcept>
#include "Game.h"

int main() {
    try {
        Game game(1920, 1080, "mr frog");
        game.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown unhandled exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}