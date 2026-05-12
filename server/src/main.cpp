#include <csignal>
#include <cstdlib>
#include <iostream>

#include "Server.h"

#define ERROR_MSG "Usage: ./server <port>"
#define EXE_KNOWN_MSG "Error: "
#define EXE_NKNOWN_MSG "Se produjo un error desconocido en el server."
#define VALID_ARGS 2
#define ARGV_PORT 1

int main(int argc, char* argv[]) try {
    std::signal(SIGPIPE, SIG_IGN);

    if (argc != VALID_ARGS) {
        std::cerr << ERROR_MSG << std::endl;
        return EXIT_FAILURE;
    }

    Server server(argv[ARGV_PORT]);
    server.run();

    return EXIT_SUCCESS;

} catch (const std::exception& e) {
    std::cerr << EXE_KNOWN_MSG << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << EXE_NKNOWN_MSG << std::endl;
    return EXIT_FAILURE;
}
