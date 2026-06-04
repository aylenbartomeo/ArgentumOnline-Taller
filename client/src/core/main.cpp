#include <QApplication>
#include <iostream>
#include <memory>

#include "../ui/Launcher.h"

#include "Client.h"
#include "Game.h"

int main(int argc, char* argv[]) try {
    std::unique_ptr<Client> activeClient = nullptr;
    {
        QApplication app(argc, argv);
        Launcher launcher;
        launcher.show();

        app.exec();

        if (!launcher.isAuthenticated()) {
            std::cout << "Launcher closure" << std::endl;
            return 0;
        }

        activeClient = launcher.releaseClient();
    }

    if (activeClient) {
        std::cout << "Starting game graphics engine (SDL)..." << std::endl;

        activeClient->start();

        Game game(*activeClient);
        game.run();

        activeClient->stop();
    }

    return 0;

} catch (const std::exception& e) {
    std::cerr << "Critical exception caught in main: " << e.what() << std::endl;
    return 1;
}
