#include <QApplication>
#include <iostream>
#include <memory>

#include "Client.h"
#include "Game.h"
#include "Launcher.h"

int main(int argc, char* argv[]) try {
    std::unique_ptr<Client> activeClient = nullptr;
    {
        QApplication app(argc, argv);
        Launcher launcher;
        launcher.show();

        app.exec();

        if (!launcher.isAuthenticated()) {
            std::cout << "Cierre del Launcher sin autenticación exitosa." << std::endl;
            return 0;
        }

        activeClient = launcher.releaseClient();
    }

    if (activeClient) {
        std::cout << "Iniciando motor gráfico del juego (SDL)..." << std::endl;

        activeClient->start();

        Game game(*activeClient);
        game.run();

        activeClient->stop();
    }

    return 0;

} catch (const std::exception& e) {
    std::cerr << "Excepción crítica en la ejecución: " << e.what() << std::endl;
    return 1;
}


/*#include <exception>
#include <iostream>
#include <string>

#include "Client.h"
#include "Game.h"

// cppcheck-suppress constParameter
int main(int argc, char* argv[]) try {
    if (argc != 6) {
        std::cerr << "Usage: ./argentum_online_client <host> <port> <login|register> <username> "
                     "<password>\n";
        return 1;
    }

    std::string host = argv[1];
    std::string port = argv[2];
    std::string action = argv[3];
    std::string username = argv[4];
    std::string password = argv[5];

    if (action != "login" && action != "register") {
        std::cerr << "Error: Action must be 'login' or 'register'.\n";
        return 1;
    }

    Client client(host.c_str(), port.c_str(), username.c_str());

    if (!client.authenticate(action, username, password)) {
        return 1;
    }

    client.start();
    Game game(client);
    game.run();
    client.stop();

    return 0;

} catch (std::exception& e) {
    std::cerr << "Critical exception caught in main: " << e.what() << std::endl;
    return 1;
}*/
