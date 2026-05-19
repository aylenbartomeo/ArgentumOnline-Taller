#include <exception>
#include <iostream>

#include "Client.h"

int main() try {
    Client client("localhost", "8080", "Aylu");
    client.start();
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
