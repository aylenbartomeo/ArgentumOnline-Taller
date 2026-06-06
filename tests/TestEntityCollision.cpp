#include <gtest/gtest.h>
#include "World.h"
#include "config/CharacterConfig.h"
#include "model/items/ItemRegistry.h"

static CharacterConfigs getTestConfigs() {
    PlayerConfig base{15, 15, 15, 15, 1, 0, 0};
    RaceConfig human{1.0f, 1.0f, 1.0f};
    CharacterClassConfig warrior{1.0f, 1.0f, 1.0f, false};
    return CharacterConfigs{base, {{Race::HUMAN, human}}, {{CharacterClass::WARRIOR, warrior}}};
}
static InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

TEST(WorldTest, PlayerCannotMoveIntoAnotherPlayer) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry, getTestConfigs(), getTestInventoryConfig());
    // Since World constructor defaults map dimensions to 20, 15, no need to do anything else.
    
    // Player 1 at 2,2
    PlayerPersistData d1{}; d1.posX = 2; d1.posY = 2; d1.hp = 15; d1.mana = 15; d1.level = 1;
    std::string n1 = "P1";
    mundo.addPlayer(1, n1, d1);
    
    // Player 2 at 3,2
    PlayerPersistData d2{}; d2.posX = 3; d2.posY = 2; d2.hp = 15; d2.mana = 15; d2.level = 1;
    std::string n2 = "P2";
    mundo.addPlayer(2, n2, d2);
    
    // Try to move Player 1 right (to 3,2)
    mundo.moveEntity(1, Movement::RIGHT);
    
    // Check positions
    auto p1 = mundo.getPlayerPosition(1);
    EXPECT_EQ(p1->x, 2); // Should not have moved!
}

