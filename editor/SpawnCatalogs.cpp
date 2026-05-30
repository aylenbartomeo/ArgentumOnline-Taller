#include "SpawnCatalogs.h"

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "1800.png", 2, 4, 24, 44},   {"orc", "1875.png", 2, 4, 24, 44},
            {"zombie", "1892.png", 2, 4, 24, 44},   {"spider", "1052.png", 2, 4, 24, 44},
            {"golem", "1140.png", 2, 4, 24, 44},    {"skeleton", "1238.png", 2, 4, 24, 44},
    };
    return catalog;
}

const std::vector<CitizenCatalogEntry>& getCitizenCatalog() {
    static const std::vector<CitizenCatalogEntry> catalog = {
            {"merchant", 'M'},
            {"banker", 'B'},
            {"priest", 'P'},
    };
    return catalog;
}

const std::vector<ItemCatalogEntry>& getItemCatalog() {
    static const std::vector<ItemCatalogEntry> catalog = {
            {1, "5137.png", 607, 232, 34, 23},
    };
    return catalog;
}
