#include "SpawnCatalogs.h"

#include <algorithm>

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "characters/monsters/goblin1.png", 2, 2, 22, 48, false, "characters/heads.png", 6, 13, 13, 15, 0},
            {"orc", "characters/monsters/orco1.png", 2, 1, 22, 60, false, "characters/heads.png", 6, 13, 13, 15, 0},
            {"zombie", "characters/monsters/zombie1.png", 5, 2, 15, 44, false, "characters/heads.png", 6, 13, 13, 15, 0},
            {"spider", "characters/monsters/spider1.png", 18, 42, 63, 54, false, "characters/heads.png", 6, 13, 13, 15, 0},
            {"golem", "characters/monsters/golem1.png", 4, 17, 47, 72, false, "characters/heads.png", 6, 13, 13, 15, 0},
            {"skeleton", "characters/monsters/skeleton1.png", 5, 3, 15, 48, false, "characters/heads.png", 6, 13, 13, 15,
             0},
    };
    return catalog;
}

const std::vector<CitizenCatalogEntry>& getCitizenCatalog() {
    static const std::vector<CitizenCatalogEntry> catalog = {
            {"merchant", "characters/npc-merchant.png", 2, 4, 24, 44, "characters/heads.png", 115, 13, 13, 15, 6},
            {"banker", "characters/npc-banker.png", 2, 4, 24, 44, "characters/heads.png", 142, 13, 13, 15, 6},
            {"priest", "characters/npc-priest.png", 2, 4, 24, 44, "characters/heads.png", 170, 13, 11, 15, 6},
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
