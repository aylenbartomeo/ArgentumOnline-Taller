#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "Editor.h"
#include "EditorMap.h"

namespace {
constexpr const char* MAP_PATH = "maps/defaultMap.json";
constexpr const char* TILESET_NAME = "tilemap_packed.png";
constexpr int DEFAULT_WIDTH = 20;
constexpr int DEFAULT_HEIGHT = 15;
constexpr int DEFAULT_TILE_SIZE = 16;
constexpr int DEFAULT_TILESET_COLS = 12;

std::string readWholeFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace

int main() {
    try {
        EditorMap map = std::filesystem::exists(MAP_PATH) ?
                EditorMap(readWholeFile(MAP_PATH)) :
                EditorMap(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_TILE_SIZE, TILESET_NAME,
                          DEFAULT_TILESET_COLS);
        Editor editor(std::move(map), MAP_PATH);
        editor.run();
    } catch (const std::exception& e) {
        std::cerr << "Error en el editor: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
