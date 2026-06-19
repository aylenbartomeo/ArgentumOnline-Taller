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
    bool drawHead;
    std::string headSheet;
    int headX;
    int headY;
    int headW;
    int headH;
    int headOverlap;
};

struct CitizenCatalogEntry {
    std::string type;
    std::string sheet;
    int srcX;
    int srcY;
    int srcW;
    int srcH;
    std::string headSheet;
    int headX;
    int headY;
    int headW;
    int headH;
    int headOverlap;
};

const std::vector<MonsterCatalogEntry>& getMonsterCatalog();
const std::vector<CitizenCatalogEntry>& getCitizenCatalog();

const MonsterCatalogEntry* monsterEntryFor(const std::string& type);
const CitizenCatalogEntry* citizenEntryFor(const std::string& type);

#endif
