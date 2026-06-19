#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "Editor.h"
#include "EditorMap.h"
#include "MapChooser.h"
#include "MapDefaults.h"

int main() {
    try {
        const std::string mapPath = MapDefaults::DEFAULT_MAP_PATH;
        EditorMap map =
                std::filesystem::exists(mapPath) ?
                        EditorMap(readMapFile(mapPath)) :
                        EditorMap(MapDefaults::WIDTH, MapDefaults::HEIGHT, MapDefaults::TILE_SIZE,
                                  MapDefaults::TILESET, MapDefaults::TILESET_COLS);
        Editor editor(std::move(map), mapPath);
        editor.run();
    } catch (const std::exception& e) {
        std::cerr << "Error en el editor: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
