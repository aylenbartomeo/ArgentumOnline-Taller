#include <exception>
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
}
