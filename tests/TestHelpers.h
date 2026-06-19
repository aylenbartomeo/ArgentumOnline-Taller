#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <cstdint>
#include <string>

#include "../server/src/config/CharacterConfig.h"
#include "../server/src/config/ServerConfigLoader.h"
#include "../server/src/model/entities/Monster.h"
#include "../server/src/model/entities/Player.h"

namespace TestUtils {

PlayerConfig getTestPlayerConfig();
RaceConfig getTestRaceConfig();
CharacterClassConfig getTestClassConfig();
InventoryConfig getTestInventoryConfig();
ServerConfig getTestServerConfig();

// Permitimos cambiar el ID y opcionalmente el nombre para tests de clanes/interacciones
Player makeTestPlayer(uint32_t id = 1, const std::string& name = "TestPlayer");

Monster makeTestMonster(uint32_t id = 2);

NpcCommandDTO createTestCommand(NpcCommandType type, const std::string& itemIdStr = "");

}  // namespace TestUtils

#endif  // TEST_HELPERS_H
