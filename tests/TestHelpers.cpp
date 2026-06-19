#include "TestHelpers.h"

#include "FormulaEngine.h"  // Si lo necesitás para el player


namespace TestUtils {

PlayerConfig getTestPlayerConfig() { return {15, 15, 15, 15, 1, 0, 0}; }
RaceConfig getTestRaceConfig() { return {1.0f, 1.0f, 1.0f}; }
CharacterClassConfig getTestClassConfig() { return {1.0f, 1.0f, 1.0f, false}; }
InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

ServerConfig getTestServerConfig() {
    ServerConfig config;
    config.worldName = "";
    config.mapPath = "";
    config.clanBonusRange = 5;
    config.criticalProbability = 0.10f;
    config.clanAttackBonusPerMember = 0.05f;
    config.clanDefenseBonusPerMember = 0.05f;
    return config;
}

Player makeTestPlayer(uint32_t id, const std::string& name) {
    Race race = Race::HUMAN;
    CharacterClass charClass = CharacterClass::WARRIOR;
    RaceConfig raceConfig = getTestRaceConfig();
    CharacterClassConfig clsConfig = getTestClassConfig();
    PlayerConfig cfg = getTestPlayerConfig();
    InventoryConfig invCfg = getTestInventoryConfig();

    return Player(id, id, name, race, charClass, raceConfig, clsConfig, cfg, invCfg,
                  FormulaEngine::getInstance());
}

Monster makeTestMonster(uint32_t id) {
    Position pos{0, 0};
    MonsterConfig mConfig = {100, 100, 0, 10, 20, 5, 2, 1, 1, "zone", 0, 0};
    return Monster(id, NPCType::GOBLIN, pos, mConfig);
}

NpcCommandDTO createTestCommand(NpcCommandType type, const std::string& itemIdStr) {
    NpcCommandDTO dto;
    dto.type = type;
    dto.arg = itemIdStr;
    return dto;
}

}  // namespace TestUtils
