#ifndef SPAWN_CATALOGS_H
#define SPAWN_CATALOGS_H

#include <string>
#include <vector>

struct MonsterCatalogEntry {
    std::string type;
    std::string sheet;
    int srcX;
    int srcY;
    int srcW;
    int srcH;
};

struct CitizenCatalogEntry {
    std::string type;
    std::string sheet;
    int srcX;
    int srcY;
    int srcW;
    int srcH;
};

const std::vector<MonsterCatalogEntry>& getMonsterCatalog();
const std::vector<CitizenCatalogEntry>& getCitizenCatalog();

#endif
