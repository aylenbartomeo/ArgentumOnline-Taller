#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>

#include <gtest/gtest.h>

#include "model/interfaces/CombatStrategies.h"
#include "model/items/ItemRegistry.h"
#include "model/items/WeaponFactory.h"
#include "persistence/PlayerDataStore.h"

#include "TestHelpers.h"
#include "World.h"

static PlayerPersistData makeSpawnData(int x, int y) {
    PlayerPersistData d{};
    d.posX = x;
    d.posY = y;
    d.hp = 15;
    d.mana = 15;
    d.level = 1;
    return d;
}

class WorldTest: public ::testing::Test {
protected:
    ItemRegistry registry{"../config/items.toml"};
    CharacterConfigs configs{TestUtils::getTestPlayerConfig(),
                             {{Race::HUMAN, TestUtils::getTestRaceConfig()}},
                             {{CharacterClass::WARRIOR, TestUtils::getTestClassConfig()}}};
    World mundo{1,
                "Tester",
                registry,
                configs,
                TestUtils::getTestInventoryConfig(),
                TestUtils::getTestServerConfig()};

    bool addPlayer(uint32_t id, const char* name, Race race = Race::HUMAN,
                   CharacterClass cls = CharacterClass::WARRIOR,
                   std::optional<PlayerPersistData> spawnData = std::nullopt) {
        std::string username{name};
        return mundo.addPlayer(id, username, race, cls, spawnData);
    }
};

// ---------------------------------------------------------------------------
// 1. INICIALIZACIÓN Y CICLO DE VIDA
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_InitializesCorrectly) {
    // Necesitamos un mundo con ID y creador específicos, distinto al del fixture
    ItemRegistry reg{"../config/items.toml"};
    CharacterConfigs cfg{TestUtils::getTestPlayerConfig(),
                         {{Race::HUMAN, TestUtils::getTestRaceConfig()}},
                         {{CharacterClass::WARRIOR, TestUtils::getTestClassConfig()}}};
    World w(42, "PaladinGM", reg, cfg, TestUtils::getTestInventoryConfig(),
            TestUtils::getTestServerConfig());

    EXPECT_EQ(w.getWorldId(), 42);
    EXPECT_EQ(w.getCreatorPlayerName(), "PaladinGM");
    EXPECT_TRUE(w.isEmpty());
    EXPECT_EQ(w.getPlayerCount(), 0);
}

TEST_F(WorldTest, LoadMapSpawnsEditorMonsters) {
    // Arrange
    const std::string mapPath = std::string("/tmp/") +
                                ::testing::UnitTest::GetInstance()->current_test_info()->name() +
                                ".json";
    {
        std::ofstream out(mapPath);
        out << R"({
            "width": 5,
            "height": 5,
            "spawn": {"x": 0, "y": 0},
            "monsters": [{"type": "goblin", "x": 2, "y": 2}]
        })";
    }

    // Act
    bool success = mundo.loadMap(mapPath, true);
    SnapshotDTO snap = mundo.generateSnapshot();

    // Assert
    ASSERT_TRUE(success);
    EXPECT_FALSE(snap.monsters.empty());

    // Cleanup
    std::filesystem::remove(mapPath);
}

// ---------------------------------------------------------------------------
// 2. JUGADORES
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_HandlesPlayerLifecycleWithUniquePtr) {
    EXPECT_TRUE(addPlayer(100, "PlayerOne", Race::HUMAN, CharacterClass::WARRIOR));
    EXPECT_TRUE(addPlayer(200, "PlayerTwo", Race::HUMAN, CharacterClass::WARRIOR));
    EXPECT_EQ(mundo.getPlayerCount(), 2);
    EXPECT_FALSE(mundo.isEmpty());

    // ID duplicado: debe rechazarlo
    EXPECT_FALSE(addPlayer(100, "PlayerThree", Race::HUMAN, CharacterClass::WARRIOR));
    EXPECT_EQ(mundo.getPlayerCount(), 2);

    EXPECT_TRUE(mundo.removePlayer(100));
    EXPECT_EQ(mundo.getPlayerCount(), 1);
    EXPECT_TRUE(mundo.removePlayer(200));
    EXPECT_TRUE(mundo.isEmpty());
}

TEST_F(WorldTest, World_RemoveNonExistentPlayerReturnsFalse) {
    EXPECT_FALSE(mundo.removePlayer(999));
}

TEST_F(WorldTest, World_PlayerCannotMoveOutsideMap) {
    ASSERT_TRUE(addPlayer(1, "EdgeWalker", Race::HUMAN, CharacterClass::WARRIOR));

    mundo.moveEntity(1, Movement::UP);
    mundo.moveEntity(1, Movement::LEFT);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 0);
    EXPECT_EQ(snap.players[0].y, 0);
}

TEST_F(WorldTest, World_PlayerCannotMoveIntoObstacle) {
    ASSERT_TRUE(addPlayer(1, "Blocker", Race::HUMAN, CharacterClass::WARRIOR));

    // Obstáculo en (1,0); jugador empieza en (0,0)
    mundo.setObstacleAt(1, 0);
    mundo.moveEntity(1, Movement::RIGHT);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 0);
    EXPECT_EQ(snap.players[0].y, 0);
}

TEST_F(WorldTest, World_GenerateSnapshotWithPlayersCorrectly) {
    SnapshotDTO snapshotInicial = mundo.generateSnapshot();
    EXPECT_TRUE(snapshotInicial.players.empty());
    EXPECT_TRUE(snapshotInicial.monsters.empty());

    ASSERT_TRUE(addPlayer(100, "Aoki", Race::HUMAN, CharacterClass::WARRIOR));
    mundo.moveEntity(100, Movement::DOWN);   // y: 0 → 1
    mundo.moveEntity(100, Movement::RIGHT);  // x: 0 → 1

    ASSERT_TRUE(addPlayer(200, "Beren", Race::HUMAN, CharacterClass::WARRIOR));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 2u);

    bool encontroPlayer1 = false;
    bool encontroPlayer2 = false;

    for (const auto& entity: snap.players) {
        if (entity.id == 100) {
            encontroPlayer1 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 1);
            EXPECT_EQ(entity.y, 1);
            EXPECT_EQ(entity.current_hp, 15);
            EXPECT_EQ(entity.max_hp, 15);
        } else if (entity.id == 200) {
            encontroPlayer2 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 0);
            EXPECT_EQ(entity.y, 0);
            EXPECT_EQ(entity.current_hp, 15);
            EXPECT_EQ(entity.max_hp, 15);
        }
    }

    EXPECT_TRUE(encontroPlayer1);
    EXPECT_TRUE(encontroPlayer2);
}

TEST_F(WorldTest, World_GetPlayerPositionReturnsCurrentPos) {
    ASSERT_TRUE(addPlayer(1, "MovingPlayer", Race::HUMAN, CharacterClass::WARRIOR));

    mundo.moveEntity(1, Movement::DOWN);
    mundo.moveEntity(1, Movement::RIGHT);

    auto pos = mundo.getPlayerPosition(1);
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 1);
    EXPECT_EQ(pos->y, 1);
}

TEST_F(WorldTest, World_GetPlayerUsernameReturnsCorrectName) {
    ASSERT_TRUE(addPlayer(42, "TestUser", Race::HUMAN, CharacterClass::WARRIOR));

    auto name = mundo.getPlayerUsername(42);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "TestUser");
}

TEST_F(WorldTest, World_GetOnlinePlayerDbIdsReturnsAllActive) {
    addPlayer(10, "u1", Race::HUMAN, CharacterClass::WARRIOR);
    addPlayer(20, "u2", Race::HUMAN, CharacterClass::WARRIOR);
    addPlayer(30, "u3", Race::HUMAN, CharacterClass::WARRIOR);

    auto ids = mundo.getOnlinePlayerDbIds();
    EXPECT_EQ(ids.size(), 3u);

    for (uint32_t id: {10u, 20u, 30u}) {
        EXPECT_NE(std::find(ids.begin(), ids.end(), id), ids.end());
    }
}

// ---------------------------------------------------------------------------
// 3. SPAWN / MAPA
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_AddPlayerWithSavedPositionSpawnsThere) {
    ASSERT_TRUE(
            addPlayer(1, "SavedPlayer", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 5);
    EXPECT_EQ(snap.players[0].y, 5);
}

TEST_F(WorldTest, World_AddPlayerWithInvalidPositionUsesDefault) {
    ASSERT_TRUE(addPlayer(1, "InvalidPosPlayer", Race::HUMAN, CharacterClass::WARRIOR,
                          makeSpawnData(-1, -1)));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    auto defaultPos = mundo.getInitialPosition();
    EXPECT_EQ(snap.players[0].x, defaultPos.first);
    EXPECT_EQ(snap.players[0].y, defaultPos.second);
}

// ---------------------------------------------------------------------------
// 4. MONSTRUOS Y COMBATE
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_UpdateTriggersMonsterAttack) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{1, 0}, mConfig);

    // Iteramos hasta que conecte (puede esquivar)
    bool tookDamage = false;
    for (int i = 0; i < 50; ++i) {
        mundo.update(0.1f);
        if (mundo.generateSnapshot().players[0].current_hp < 15) {
            tookDamage = true;
            break;
        }
    }

    EXPECT_TRUE(tookDamage);
}

TEST_F(WorldTest, World_UpdateDoesNotTriggerMonsterAttackIfOutOfRange) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 1, 1, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{3, 0}, mConfig);  // dist = 3, rango = 1

    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    ASSERT_EQ(snap.monsters.size(), 1u);

    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.type == EntityType::PLAYER; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);
}

TEST_F(WorldTest, World_MonsterDropsLootOnDeath_AndCleanup) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));
    mundo.setFairPlayRules(false);

    Player* p = mundo.getPlayerById(1);
    p->applyBoost(BoostType::STRENGTH, 100, 10000);

    Weapon testSword(999, "Espada", 100, WeaponType::MELEE, 50, 100, 2, 0,
                     WeaponFactory::createDeliveryStrategy(WeaponType::MELEE),
                     WeaponFactory::createHitEffectStrategy(WeaponType::MELEE));
    p->equipWeapon(&testSword);

    bool droppedSomething = false;
    for (int i = 0; i < 500; i++) {
        MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, 10, "zone", 0, 0};
        uint32_t mId = mundo.addMonster(NPCType::GOBLIN, Position{5, 6}, mConfig);

        mundo.playerAttack(1, mId);
        mundo.update(33.0f);

        SnapshotDTO snap = mundo.generateSnapshot();
        EXPECT_TRUE(snap.monsters.empty());

        if (!snap.groundItems.empty()) {
            droppedSomething = true;
            break;
        }
    }

    EXPECT_TRUE(droppedSomething);
}

// ---------------------------------------------------------------------------
// 5. ZONAS SEGURAS
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_IsSafeZone_delegates_to_map) {
    EXPECT_TRUE(mundo.isSafeZone(50, 50));
    EXPECT_FALSE(mundo.isSafeZone(0, 0));
}

TEST_F(WorldTest, World_PlayerCannotAttackInSafeZone) {
    ASSERT_TRUE(
            addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(50, 50)));
    ASSERT_TRUE(
            addPlayer(2, "Player2", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(51, 50)));

    mundo.playerAttack(1, 2);

    SnapshotDTO snap = mundo.generateSnapshot();
    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.id == 2; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);
}

TEST_F(WorldTest, World_MonsterCannotAttackInSafeZone) {
    ASSERT_TRUE(
            addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(50, 50)));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{51, 50}, mConfig);

    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.id == 1; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);
}

TEST_F(WorldTest, World_MonsterLosesAggroInSafeZone) {
    ASSERT_TRUE(
            addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(50, 50)));

    // Monstruo fuera de la zona segura (44,50) pero dentro del rango de detección
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{44, 50}, mConfig);

    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    auto itM = std::find_if(snap.monsters.begin(), snap.monsters.end(),
                            [](const EntityDTO& e) { return e.type == EntityType::MONSTER; });
    ASSERT_NE(itM, snap.monsters.end());

    // No debió moverse hacia el jugador
    EXPECT_EQ(itM->x, 44);
    EXPECT_EQ(itM->y, 50);
}

// ---------------------------------------------------------------------------
// 6. LÍNEA DE VISIÓN EN COMBATE
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_PlayerCannotAttackThroughObstacle_Straight) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, 10, "zone", 0, 0};
    uint32_t monsterId = mundo.addMonster(NPCType::GOBLIN, Position{9, 5}, mConfig);

    mundo.setObstacleAt(7, 5);
    mundo.playerAttack(1, monsterId);

    auto evs = mundo.pollEvents();
    bool blockedEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Hay un obstaculo bloqueando tu vision.";
    });
    EXPECT_TRUE(blockedEvent);
}

TEST_F(WorldTest, World_PlayerCannotAttackThroughObstacle_Diagonal) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, 10, "zone", 0, 0};
    uint32_t monsterId = mundo.addMonster(NPCType::GOBLIN, Position{9, 9}, mConfig);

    mundo.setObstacleAt(7, 7);
    mundo.playerAttack(1, monsterId);

    auto evs = mundo.pollEvents();
    bool blockedEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Hay un obstaculo bloqueando tu vision.";
    });
    EXPECT_TRUE(blockedEvent);
}

// ---------------------------------------------------------------------------
// 7. SISTEMA DE ÍTEMS (Pick Up & Drop)
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_PlaceAndPickUpItemOnGround) {
    Position pos{5, 5};
    EXPECT_TRUE(mundo.placeItemOnGround(pos, 1, 10));

    auto picked = mundo.pickUpItemFromGround(pos);
    ASSERT_TRUE(picked.has_value());
    EXPECT_EQ(picked->itemId, 1);
    EXPECT_EQ(picked->amount, 10);
}

TEST_F(WorldTest, World_SnapshotIncludesGroundItems) {
    mundo.placeItemOnGround(Position{2, 2}, 1, 5);
    mundo.placeItemOnGround(Position{3, 3}, 2, 1);

    SnapshotDTO snap = mundo.generateSnapshot();
    EXPECT_EQ(snap.groundItems.size(), 2u);

    bool foundItem1 = false;
    for (const auto& gi: snap.groundItems) {
        if (gi.itemId == 1) {
            EXPECT_EQ(gi.x, 2);
            EXPECT_EQ(gi.y, 2);
            EXPECT_EQ(gi.amount, 5);
            foundItem1 = true;
        }
    }
    EXPECT_TRUE(foundItem1);
}

TEST_F(WorldTest, World_PickUpItemIntoInventory) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);

    EXPECT_TRUE(mundo.generateSnapshot().groundItems.empty());

    auto evs = mundo.pollEvents();
    bool pickedUpEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Objeto recogido.";
    });
    EXPECT_TRUE(pickedUpEvent);
}

TEST_F(WorldTest, World_PickUpItemNothingToPickUp) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    mundo.pickUpItem(1);

    auto evs = mundo.pollEvents();
    bool nothingHereEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "No hay objetos aquí para recoger.";
    });
    EXPECT_TRUE(nothingHereEvent);
}

TEST_F(WorldTest, World_PickUpItemNoSpaceInInventory) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    for (int i = 0; i < 20; ++i) {
        mundo.placeItemOnGround(Position{5, 5}, 100 + i, 1);
        mundo.pickUpItem(1);
    }
    mundo.pollEvents();

    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);

    auto snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.groundItems.size(), 1u);
    EXPECT_EQ(snap.groundItems[0].itemId, 202u);
    EXPECT_EQ(snap.groundItems[0].amount, 10);

    auto evs = mundo.pollEvents();
    bool fullEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Inventario lleno. No pudiste recoger todo.";
    });
    EXPECT_TRUE(fullEvent);
}

TEST_F(WorldTest, World_DropItemSuccess) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);
    mundo.dropItem(1, 0, 5);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.groundItems.size(), 1u);
    EXPECT_EQ(snap.groundItems[0].itemId, 202u);
    EXPECT_EQ(snap.groundItems[0].amount, 5);
}

TEST_F(WorldTest, World_DropItemDynamicSearchSucceedsWhenCenterFull) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);

    // Llenamos el 3x3 alrededor del jugador
    for (auto [px, py]: std::initializer_list<std::pair<int, int>>{
                 {5, 5}, {5, 4}, {5, 6}, {4, 5}, {6, 5}, {4, 4}, {6, 6}, {4, 6}, {6, 4}}) {
        mundo.placeItemOnGround(Position{px, py}, 101, 1);
    }
    mundo.pollEvents();

    mundo.dropItem(1, 0, 5);

    auto snap = mundo.generateSnapshot();
    // 9 de oro (ID 101) + 1 el nuevo dropeado
    ASSERT_EQ(snap.groundItems.size(), 10u);

    bool foundNuevo = false;
    for (const auto& gItem: snap.groundItems) {
        if (gItem.itemId == 202u && gItem.amount == 5) {
            foundNuevo = true;
            EXPECT_TRUE(gItem.x < 4 || gItem.x > 6 || gItem.y < 4 || gItem.y > 6);
        }
    }
    EXPECT_TRUE(foundNuevo);
}

// ---------------------------------------------------------------------------
// 8. MUERTE Y LOOT
// ---------------------------------------------------------------------------

TEST_F(WorldTest, World_PlayerDeathDropsInventoryItems) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);
    mundo.placeItemOnGround(Position{5, 5}, 101, 1);
    mundo.pickUpItem(1);
    mundo.placeItemOnGround(Position{5, 5}, 102, 1);
    mundo.pickUpItem(1);

    mundo.handlePlayerDeath(1);

    auto snap = mundo.generateSnapshot();
    EXPECT_GE(snap.groundItems.size(), 3u);
}

TEST_F(WorldTest, World_PlayerDeathDropsExcessGold) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    Player* player = mundo.getPlayerById(1);
    ASSERT_NE(player, nullptr);
    player->addGold(10000);  // Supera el safe limit de 5000

    mundo.handlePlayerDeath(1);

    auto snap = mundo.generateSnapshot();

    bool foundGold = false;
    for (const auto& item: snap.groundItems) {
        if (item.itemId == 1) {  // GOLD_ITEM_ID
            EXPECT_EQ(item.amount, 5000);
            foundGold = true;
        }
    }
    EXPECT_TRUE(foundGold);
    EXPECT_EQ(player->getGold(), 5000);
}

TEST_F(WorldTest, World_PickUpGoldAddsToWallet) {
    ASSERT_TRUE(addPlayer(1, "Player1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(5, 5)));

    Player* player = mundo.getPlayerById(1);
    ASSERT_NE(player, nullptr);
    uint32_t initialGold = player->getGold();

    mundo.placeItemOnGround(Position{5, 5}, 1, 1500);  // 1 = GOLD_ITEM_ID
    mundo.pickUpItem(1);

    EXPECT_EQ(player->getGold(), initialGold + 1500);

    auto evs = mundo.pollEvents();
    bool pickedUpEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Recogiste 1500 monedas de oro.";
    });
    EXPECT_TRUE(pickedUpEvent);
}


TEST_F(WorldTest, PlayerCannotMoveIntoAnotherPlayer) {
    addPlayer(1, "P1", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(2, 2));
    addPlayer(2, "P2", Race::HUMAN, CharacterClass::WARRIOR, makeSpawnData(3, 2));

    // Player 1 intenta moverse a la derecha hacia (3,2) donde está Player 2 -> colisión
    mundo.moveEntity(1, Movement::RIGHT);

    auto p1 = mundo.getPlayerPosition(1);
    EXPECT_EQ(p1->x, 2);
}
