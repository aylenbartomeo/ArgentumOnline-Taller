#include "SpawnCatalogs.h"

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "1800.png", 2, 4, 24, 44}, {"orc", "1875.png", 2, 4, 24, 44},
            {"zombie", "1892.png", 2, 4, 24, 44}, {"spider", "1052.png", 2, 4, 24, 44},
            {"golem", "1140.png", 2, 4, 24, 44},  {"skeleton", "1238.png", 2, 4, 24, 44},
    };
    return catalog;
}

const std::vector<CitizenCatalogEntry>& getCitizenCatalog() {
    static const std::vector<CitizenCatalogEntry> catalog = {
            {"merchant", "1077.png", 2, 4, 24, 44},
            {"banker", "1071.png", 2, 4, 24, 44},
            {"priest", "1910.png", 2, 4, 24, 44},
    };
    return catalog;
}
