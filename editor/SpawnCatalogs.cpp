#include "SpawnCatalogs.h"

#include <algorithm>

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "monsters/goblin1.png", 2, 2, 22, 48, false, "420.png", 6, 13, 13, 15, 0},
            {"orc", "monsters/orco1.png", 2, 1, 22, 60, false, "420.png", 6, 13, 13, 15, 0},
            {"zombie", "monsters/zombie1.png", 5, 2, 15, 44, false, "420.png", 6, 13, 13, 15, 0},
            {"spider", "monsters/spider1.png", 18, 42, 63, 54, false, "420.png", 6, 13, 13, 15, 0},
            {"golem", "monsters/golem1.png", 4, 17, 47, 72, false, "420.png", 6, 13, 13, 15, 0},
            {"skeleton", "monsters/skeleton1.png", 5, 3, 15, 48, false, "420.png", 6, 13, 13, 15,
             0},
    };
    return catalog;
}

const std::vector<CitizenCatalogEntry>& getCitizenCatalog() {
    static const std::vector<CitizenCatalogEntry> catalog = {
            {"merchant", "1077.png", 2, 4, 24, 44, "420.png", 115, 13, 13, 15, 6},
            {"banker", "1071.png", 2, 4, 24, 44, "420.png", 142, 13, 13, 15, 6},
            {"priest", "1910.png", 2, 4, 24, 44, "420.png", 170, 13, 11, 15, 6},
    };
    return catalog;
}

const MonsterCatalogEntry* monsterEntryFor(const std::string& type) {
    const std::vector<MonsterCatalogEntry>& catalog = getMonsterCatalog();
    auto it = std::find_if(catalog.begin(), catalog.end(),
                           [&type](const MonsterCatalogEntry& e) { return e.type == type; });
    return (it != catalog.end()) ? &(*it) : nullptr;
}

const CitizenCatalogEntry* citizenEntryFor(const std::string& type) {
    const std::vector<CitizenCatalogEntry>& catalog = getCitizenCatalog();
    auto it = std::find_if(catalog.begin(), catalog.end(),
                           [&type](const CitizenCatalogEntry& e) { return e.type == type; });
    return (it != catalog.end()) ? &(*it) : nullptr;
}
