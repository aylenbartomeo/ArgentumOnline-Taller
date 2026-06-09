#include "SpawnCatalogs.h"

const std::vector<MonsterCatalogEntry>& getMonsterCatalog() {
    static const std::vector<MonsterCatalogEntry> catalog = {
            {"goblin", "1800.png", 2, 4, 24, 44, true, "422.png", 195, 14, 15, 14, 6},
            {"orc", "1875.png", 2, 4, 24, 44, true, "430.png", 6, 16, 15, 13, 15},
            {"zombie", "1892.png", 2, 4, 24, 44, true, "420.png", 89, 13, 11, 15, 6},
            {"spider", "1052.png", 4, 5, 19, 39, false, "420.png", 6, 13, 13, 15, 0},
            {"golem", "1140.png", 3, 16, 21, 30, false, "420.png", 6, 13, 13, 15, 0},
            {"skeleton", "152.png", 4, 4, 22, 38, false, "420.png", 6, 13, 13, 15, 0},
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
