#include <fstream>

#include "CityStamp.h"
#include "EditorMap.h"
#include "TerrainRegistry.h"

int main() {
    EditorMap map(60, 60, 32, "5108.png", 32);

    applyCityPrefab(map, 5, 5, "Nix");
    applyCityPrefab(map, 25, 25, "Ullathorpe");
    applyCityPrefab(map, 45, 45, "Banderbill");

    map.setSpawn(6, 9);

    std::ofstream out("maps/defaultMap.json");
    out << map.toJson();
    return 0;
}
