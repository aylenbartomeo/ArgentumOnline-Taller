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
#define VALID_ARGS 2
#define ARGV_PORT 1

int main(int argc, char* argv[]) try {
    std::signal(SIGPIPE, SIG_IGN);

    if (argc != VALID_ARGS) {
        std::cerr << ERROR_MSG << std::endl;
        return EXIT_FAILURE;
    }

    WorldDataStore wds("worlds/");
    WorldConfig wConfig;

    std::cout << "\n=== Servidor Argentum Online ===\n";
    std::cout << "\nSeleccione una opcion:\n";
    std::cout << "\n  1) Nuevo mundo\n";
    std::cout << "\n  2) Cargar mundo\n";
    std::cout << "\n> ";

    std::string choice;
    if (!std::getline(std::cin, choice)) {
        return EXIT_FAILURE;
    }

    if (choice == "1") {
        std::vector<std::string> availableMaps;
        try {
            for (const auto& entry: std::filesystem::directory_iterator("maps/")) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    availableMaps.push_back(entry.path().filename().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "\nError accediendo al directorio maps/: " << e.what() << "\n";
            return EXIT_FAILURE;
        }

        if (availableMaps.empty()) {
            std::cerr << "\nNo se encontraron mapas en el directorio maps/.\n";
            return EXIT_FAILURE;
        }

        std::cout << "\nMapas disponibles:\n";
        for (size_t i = 0; i < availableMaps.size(); ++i) {
            std::cout << "\n  " << i + 1 << ") " << availableMaps[i] << "\n";
        }
        std::cout << "\nSeleccionar mapa: ";
        std::string mapChoice;
        std::getline(std::cin, mapChoice);

        int mapIdx = 0;
        try {
            mapIdx = std::stoi(mapChoice) - 1;
        } catch (...) {
            std::cerr << "\nOpcion invalida.\n";
            return EXIT_FAILURE;
        }

        if (mapIdx < 0 || mapIdx >= static_cast<int>(availableMaps.size())) {
            std::cerr << "\nOpcion invalida.\n";
            return EXIT_FAILURE;
        }

        std::string mapPath = "maps/" + availableMaps[mapIdx];

        std::cout << "\nIngresar nombre del mundo: ";
        std::string worldName;
        std::getline(std::cin, worldName);

        uint32_t worldId = wds.createWorld(worldName, mapPath);
        std::cout << "\n[Servidor] Mundo #" << worldId << " '" << worldName << "' creado desde '"
                  << mapPath << "'\n";

        wConfig.worldId = worldId;
        wConfig.worldName = worldName;
        wConfig.baseMapPath = mapPath;
        wConfig.worldDir = wds.getWorldDir(worldId);
        wConfig.isNewWorld = true;
    } else if (choice == "2") {
        auto worlds = wds.listSavedWorlds();
        if (worlds.empty()) {
            std::cerr << "\nNo se encontraron mundos guardados.\n";
            return EXIT_FAILURE;
        }
        std::cout << "\nMundos guardados:\n";
        for (size_t i = 0; i < worlds.size(); ++i) {
            std::cout << "\n  " << i + 1 << ") #" << worlds[i].worldId << " - " << worlds[i].name
                      << "\n";
        }
        std::cout << "\nSeleccionar mundo: ";
        std::string wChoice;
        std::getline(std::cin, wChoice);
        int idx = 0;
        try {
            idx = std::stoi(wChoice) - 1;
        } catch (...) {
            std::cerr << "\nOpcion invalida.\n";
            return EXIT_FAILURE;
        }
        if (idx < 0 || idx >= static_cast<int>(worlds.size())) {
            std::cerr << "\nOpcion invalida.\n";
            return EXIT_FAILURE;
        }
        auto selected = worlds[idx];
        std::cout << "\n[Servidor] Mundo #" << selected.worldId << " '" << selected.name << "'\n";

        wConfig.worldId = selected.worldId;
        wConfig.worldName = selected.name;
        wConfig.baseMapPath = selected.baseMapPath;
        wConfig.worldDir = wds.getWorldDir(selected.worldId);
        wConfig.isNewWorld = false;
    } else {
        std::cerr << "\nOpcion invalida.\n";
        return EXIT_FAILURE;
    }

    Server server(argv[ARGV_PORT], wConfig);
    server.run();

    return EXIT_SUCCESS;

} catch (const std::exception& e) {
    std::cerr << EXE_KNOWN_MSG << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << EXE_NKNOWN_MSG << std::endl;
    return EXIT_FAILURE;
}
