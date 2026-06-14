#include <QApplication>
#include <iostream>
#include <memory>

#include "../ui/Launcher.h"

#include "Client.h"
#include "Game.h"

int main(int argc, char* argv[]) try {
    QApplication app(argc, argv);

    while (true) {
        std::unique_ptr<Client> activeClient = nullptr;
        {
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
