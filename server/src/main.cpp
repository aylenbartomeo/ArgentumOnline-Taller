#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "persistence/WorldDataStore.h"

#include "Server.h"

#define ERROR_MSG "Uso: ./server <puerto>"
#define EXE_KNOWN_MSG "Error: "
#define EXE_NKNOWN_MSG "Ha ocurrido un error desconocido en el servidor."
#define ARGV_PORT 1

int main(int argc, char* argv[]) try {
    std::signal(SIGPIPE, SIG_IGN);

    if (argc < 4) {
        std::cerr << "Uso:\n";
        std::cerr
                << "  Para crear: ./server <puerto> --create <nombre_mundo> [--map <ruta_mapa>]\n";
        std::cerr << "  Para cargar: ./server <puerto> --load <nombre_mundo>\n";
        return EXIT_FAILURE;
    }

    WorldDataStore wds("worlds/");
    WorldConfig wConfig;

    std::string port = argv[1];
    std::string command = argv[2];
    std::string worldName = argv[3];

    if (command == "--create") {
        std::string mapPath = "maps/defaultMap.json";

        if (argc >= 6 && std::string(argv[4]) == "--map") {
            mapPath = argv[5];
        }

        uint32_t worldId = wds.createWorld(worldName, mapPath);
        std::cout << "\n[Servidor] Mundo #" << worldId << " '" << worldName << "' creado desde '"
                  << mapPath << "'\n";

        wConfig.worldId = worldId;
        wConfig.worldName = worldName;
        wConfig.baseMapPath = mapPath;
        wConfig.worldDir = wds.getWorldDir(worldId);
        wConfig.isNewWorld = true;
    } else if (command == "--load") {
        auto worlds = wds.listSavedWorlds();

        auto it = std::find_if(worlds.begin(), worlds.end(),
                               [&worldName](const auto& w) { return w.name == worldName; });

        bool found = (it != worlds.end());
        if (found) {
            wConfig.worldId = it->worldId;
            wConfig.worldName = it->name;
            wConfig.baseMapPath = it->baseMapPath;
            wConfig.worldDir = wds.getWorldDir(it->worldId);
            wConfig.isNewWorld = false;
            std::cout << "\n[Servidor] Cargando Mundo #" << it->worldId << " '" << it->name
                      << "'\n";
        }

        if (!found) {
            std::cerr << "Error: No se encontro ningun mundo guardado con el nombre '" << worldName
                      << "'.\n";
            std::cerr << "Mundos disponibles:\n";
            for (const auto& w: worlds) {
                std::cerr << "  - " << w.name << " (ID: " << w.worldId << ")\n";
            }
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << "Comando invalido: " << command << "\n";
        return EXIT_FAILURE;
    }

    Server server(port.c_str(), wConfig);
    server.run();

    return EXIT_SUCCESS;

} catch (const std::exception& e) {
    std::cerr << EXE_KNOWN_MSG << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << EXE_NKNOWN_MSG << std::endl;
    return EXIT_FAILURE;
}
