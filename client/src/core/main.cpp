#include <iostream>
#include <memory>

#include "../ui/Launcher.h"

#include "Client.h"
#include "Game.h"
int main(int argc, const char* argv[]) try {
    int windowW = 1024;
    int windowH = 768;
    bool fullscreen = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--fullscreen" || arg == "-f") {
            fullscreen = true;
        } else if (arg == "--width" && i + 1 < argc) {
            windowW = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            windowH = std::stoi(argv[++i]);
        }
    }

    while (true) {
        std::unique_ptr<Client> activeClient = nullptr;
        {
            Launcher launcher(windowW, windowH, fullscreen);
            launcher.run();

            if (!launcher.isAuthenticated()) {
                std::cout << "Launcher closure" << std::endl;
                return 0;
            }

            activeClient = launcher.releaseClient();
        }

        if (activeClient) {
            std::cout << "Starting game graphics engine (SDL)..." << std::endl;

            activeClient->start();

            Game game(*activeClient, windowW, windowH, fullscreen);
            bool goToLogin = game.runStartupAndCreation();

            activeClient->stop();

            if (!goToLogin) {
                break;
            }
        } else {
            break;
        }
    }

    return 0;

} catch (const std::exception& e) {
    std::cerr << "Critical exception caught in main: " << e.what() << std::endl;
    return 1;
}
