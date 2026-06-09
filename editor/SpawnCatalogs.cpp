#include "SpawnCatalogs.h"

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "4015.png", 2, 2, 22, 48, false, "420.png", 6, 13, 13, 15, 0},
            {"orc", "4017.png", 2, 1, 22, 60, false, "420.png", 6, 13, 13, 15, 0},
            {"zombie", "1892.png", 2, 4, 24, 44, true, "420.png", 89, 13, 11, 15, 9},
            {"spider", "4151.png", 18, 42, 63, 54, false, "420.png", 6, 13, 13, 15, 0},
            {"golem", "4091.png", 4, 17, 47, 72, false, "420.png", 6, 13, 13, 15, 0},
            {"skeleton", "4079.png", 5, 3, 15, 48, false, "420.png", 6, 13, 13, 15, 0},
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
