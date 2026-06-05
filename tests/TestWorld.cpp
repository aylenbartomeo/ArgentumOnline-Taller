#include <algorithm>
#include <fstream>
#include <memory>

#include <gtest/gtest.h>

#include "config/CharacterConfig.h"
#include "model/entities/Player.h"
#include "model/items/ItemRegistry.h"
#include "persistence/PlayerDataStore.h"

#include "World.h"

static CharacterConfigs getTestConfigs() {
    PlayerConfig base{15, 15, 15, 15, 1, 0, 0};
    RaceConfig human{1.0f, 1.0f, 1.0f};
    CharacterClassConfig warrior{1.0f, 1.0f, 1.0f, false};
    return CharacterConfigs{base, {{Race::HUMAN, human}}, {{CharacterClass::WARRIOR, warrior}}};
}

static InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

static PlayerPersistData makeSpawnData(int x, int y) {
    PlayerPersistData d{};
    d.posX = x;
    d.posY = y;
    d.hp = 15;
    d.mana = 15;
    d.level = 1;
    return d;
}

TEST(WorldTest, World_InitializesCorrectly) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(42, "PaladinGM", registry, configs, getTestInventoryConfig());

    EXPECT_EQ(mundo.getWorldId(), 42);
    EXPECT_EQ(mundo.getCreatorPlayerName(), "PaladinGM");
    EXPECT_TRUE(mundo.isEmpty());
    EXPECT_EQ(mundo.getPlayerCount(), 0);
}

TEST(WorldTest, World_HandlesPlayerLifecycleWithUniquePtr) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "ServerAdmin", registry, configs, getTestInventoryConfig());

    uint32_t id1 = 100;
    std::string username1 = "PlayerOne";
    uint32_t id2 = 200;
    std::string username2 = "PlayerTwo";

    // 2. Se los transferimos al mundo mediante std::move
    EXPECT_TRUE(mundo.addPlayer(id1, username1));
    EXPECT_TRUE(mundo.addPlayer(id2, username2));

    EXPECT_EQ(mundo.getPlayerCount(), 2);
    EXPECT_FALSE(mundo.isEmpty());

    // Verificamos que no permite duplicar IDs lógicos
    // Intentamos meter otro jugador con un ID que ya existe (100)
    uint32_t id3 = 100;
    std::string username3 = "PlayerThree";
    EXPECT_FALSE(mundo.addPlayer(id3, username3));
    EXPECT_EQ(mundo.getPlayerCount(), 2);  // Sigue teniendo 2 originales

    // Remoción y limpieza del mapa
    EXPECT_TRUE(mundo.removePlayer(id1));
    EXPECT_EQ(mundo.getPlayerCount(), 1);

    EXPECT_TRUE(mundo.removePlayer(id2));
    EXPECT_TRUE(mundo.isEmpty());
}

TEST(WorldTest, World_PlayerCannotMoveOutsideMap) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "EdgeWalker";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    mundo.moveEntity(1, Movement::UP);
    mundo.moveEntity(1, Movement::LEFT);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 0);
    EXPECT_EQ(snap.players[0].y, 0);
}

TEST(WorldTest, World_RemoveNonExistentPlayerReturnsFalse) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Intentar sacar a alguien de un mundo vacío no debería romper nada
    EXPECT_FALSE(mundo.removePlayer(999));
}

TEST(WorldTest, World_GenerateSnapshotWithPlayersCorrectly) {
    // 1. Inicializamos un mundo vacío
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(42, "PaladinGM", registry, configs, getTestInventoryConfig());

    // Verificamos que el snapshot inicial esté vacío
    SnapshotDTO snapshotInicial = mundo.generateSnapshot();
    EXPECT_TRUE(snapshotInicial.players.empty());
    EXPECT_TRUE(snapshotInicial.monsters.empty());

    // 2. Simulamos el login de dos jugadores (gatilla la creación de Players)
    std::string user1 = "Aoki";
    std::string user2 = "Beren";

    ASSERT_TRUE(mundo.addPlayer(100, user1));
    ASSERT_TRUE(mundo.addPlayer(200, user2));

    // 3. Modificamos la posición de uno para testear que el snapshot arrastre datos vivos
    // (Simula un comando de movimiento previo al snapshot)
    mundo.moveEntity(100, Movement::DOWN);   // y: 0 -> 1
    mundo.moveEntity(100, Movement::RIGHT);  // x: 0 -> 1

    // 4. Generamos el Snapshot que se le enviaría al cliente
    SnapshotDTO snapshotActual = mundo.generateSnapshot();

    // 5. Validaciones de la estructura del SnapshotDTO
    ASSERT_EQ(snapshotActual.players.size(), 2);

    bool encontroPlayer1 = false;
    bool encontroPlayer2 = false;
    int spritesEvaluados = 0;

    for (const auto& entity: snapshotActual.players) {
        spritesEvaluados++;  // El primero que salga se lleva el 1, el segundo el 2
        std::cout << "Entity ID in snapshot: " << entity.id << std::endl;

        if (entity.id == 100) {
            encontroPlayer1 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 1);
            EXPECT_EQ(entity.y, 1);
            EXPECT_EQ(entity.current_hp, 15);
            EXPECT_EQ(entity.max_hp, 15);

            // Validamos que su sprite coincida con el orden de salida real en el loop
            EXPECT_EQ(entity.sprite_id, spritesEvaluados);
        } else if (entity.id == 200) {
            encontroPlayer2 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 0);
            EXPECT_EQ(entity.y, 0);
            EXPECT_EQ(entity.current_hp, 15);
            EXPECT_EQ(entity.max_hp, 15);

            // Validamos que su sprite coincida con el orden de salida real en el loop
            EXPECT_EQ(entity.sprite_id, spritesEvaluados);
        }
    }

    EXPECT_TRUE(encontroPlayer1);
    EXPECT_TRUE(encontroPlayer2);
}

TEST(WorldTest, World_PlayerCannotMoveIntoObstacle) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Blocker";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // El jugador empieza en (0,0). Ponemos obstáculo en (1,0).
    mundo.setObstacleAt(1, 0);

    mundo.moveEntity(1, Movement::RIGHT);  // Intentar ir a (1,0) - bloqueado

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 0);  // No se movió
    EXPECT_EQ(snap.players[0].y, 0);
}

TEST(WorldTest, World_UpdateTriggersMonsterAttack) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // Config de monstruo con rango de ataque suficiente
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, "zone", 0, 0};
    Position mPos = {1, 0};  // Player está en {0, 0}
    mundo.addMonster(NPCType::GOBLIN, mPos, mConfig);

    // Player en {0,0}, Monster en {1,0}, distancia es 1.
    // Rango de ataque del monstruo es 2. Debería atacar.
    // Vida base del player es 15. Puede esquivar (63% chance), así que iteramos
    // hasta que el golpe conecte para evitar flaky tests.
    bool tookDamage = false;
    for (int i = 0; i < 50; ++i) {
        mundo.update(0.1f);  // 0.1f para minimizar regeneración pasiva
        SnapshotDTO snap = mundo.generateSnapshot();
        if (snap.players[0].current_hp < 15) {
            tookDamage = true;
            break;
        }
    }

    EXPECT_TRUE(tookDamage);
}

TEST(WorldTest, World_UpdateDoesNotTriggerMonsterAttackIfOutOfRange) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // Config de monstruo con rango de ataque pequeño
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 1, 1, "zone", 0, 0};
    Position mPos = {3, 0};  // Player está en {0, 0}, dist = 3.
    mundo.addMonster(NPCType::GOBLIN, mPos, mConfig);

    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    ASSERT_EQ(snap.monsters.size(), 1u);

    // Find player in snapshot
    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.type == EntityType::PLAYER; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);  // NO recibió daño
}

TEST(WorldTest, World_AddPlayerSpawnsAtMapSpawn) {
    const std::string path = "/tmp/test_world_spawn.json";
    std::ofstream out(path);
    out << R"({"tileSize":16,"tileset":"x.png","tilesetCols":12,"width":20,"height":15,)"
        << R"("spawn":{"x":3,"y":4},"tiles":[]})";
    out.close();

    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    ASSERT_TRUE(mundo.loadMap(path));

    std::string user = "Spawner";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 3);
    EXPECT_EQ(snap.players[0].y, 4);
}

TEST(WorldTest, World_AddPlayerWithSavedPositionSpawnsThere) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "SavedPlayer";
    Position savedPos{5, 5};
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(savedPos.x, savedPos.y)));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, 5);
    EXPECT_EQ(snap.players[0].y, 5);
}

TEST(WorldTest, World_AddPlayerWithInvalidPositionUsesDefault) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "InvalidPosPlayer";
    Position invalidPos{-1, -1};  // assuming this is out of bounds
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(invalidPos.x, invalidPos.y)));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    auto defaultPos = mundo.getInitialPosition();
    EXPECT_EQ(snap.players[0].x, defaultPos.first);
    EXPECT_EQ(snap.players[0].y, defaultPos.second);
}

TEST(WorldTest, World_GetPlayerPositionReturnsCurrentPos) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "MovingPlayer";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    mundo.moveEntity(1, Movement::DOWN);
    mundo.moveEntity(1, Movement::RIGHT);

    auto pos = mundo.getPlayerPosition(1);
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos->x, 1);
    EXPECT_EQ(pos->y, 1);
}

TEST(WorldTest, World_GetPlayerUsernameReturnsCorrectName) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "TestUser";
    ASSERT_TRUE(mundo.addPlayer(42, user));

    auto name = mundo.getPlayerUsername(42);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "TestUser");
}

TEST(WorldTest, World_GetOnlinePlayerDbIdsReturnsAllActive) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string u1 = "u1";
    std::string u2 = "u2";
    std::string u3 = "u3";
    mundo.addPlayer(10, u1);
    mundo.addPlayer(20, u2);
    mundo.addPlayer(30, u3);

    auto ids = mundo.getOnlinePlayerDbIds();
    EXPECT_EQ(ids.size(), 3u);

    // Convert to vector and check if they exist
    std::vector<uint32_t> expected = {10, 20, 30};
    for (auto id: expected) {
        EXPECT_NE(std::find(ids.begin(), ids.end(), id), ids.end());
    }
}

TEST(WorldTest, World_PlaceAndPickUpItemOnGround) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    Position pos{5, 5};
    EXPECT_TRUE(mundo.placeItemOnGround(pos, 1, 10));

    auto picked = mundo.pickUpItemFromGround(pos);
    ASSERT_TRUE(picked.has_value());
    EXPECT_EQ(picked->itemId, 1);
    EXPECT_EQ(picked->amount, 10);
}

TEST(WorldTest, World_SnapshotIncludesGroundItems) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

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

TEST(WorldTest, World_IsSafeZone_delegates_to_map) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Por defecto Map tiene una safe zone en 45, 45, 10x10
    EXPECT_TRUE(mundo.isSafeZone(50, 50));
    EXPECT_FALSE(mundo.isSafeZone(0, 0));
}

TEST(WorldTest, World_PlayerCannotAttackInSafeZone) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Player 1 in safe zone (50, 50)
    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(50, 50)));

    // Player 2 in safe zone (51, 50)
    std::string p2 = "Player2";
    ASSERT_TRUE(mundo.addPlayer(2, p2, makeSpawnData(51, 50)));

    // Attack should fail
    mundo.playerAttack(1, 2);

    SnapshotDTO snap = mundo.generateSnapshot();
    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.id == 2; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);  // HP should be intact
}

TEST(WorldTest, World_MonsterCannotAttackInSafeZone) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Player 1 in safe zone (50, 50)
    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(50, 50)));

    // Monster in safe zone (51, 50)
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{51, 50}, mConfig);

    // Update should not trigger attack
    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    auto it = std::find_if(snap.players.begin(), snap.players.end(),
                           [](const EntityDTO& e) { return e.id == 1; });
    ASSERT_NE(it, snap.players.end());
    EXPECT_EQ(it->current_hp, 15);  // HP should be intact
}

TEST(WorldTest, World_MonsterLosesAggroInSafeZone) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Player 1 in safe zone (50, 50)
    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(50, 50)));

    // Monster OUTSIDE safe zone but in detection range (44, 50) -> dist = 6 (range 10)
    // Safe zone is 45 to 54. So 44 is outside.
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, "zone", 0, 0};
    mundo.addMonster(NPCType::GOBLIN, Position{44, 50}, mConfig);

    mundo.update(1.0f);

    // Monster should not have moved towards the player because it loses aggro
    SnapshotDTO snap = mundo.generateSnapshot();
    auto itM = std::find_if(snap.monsters.begin(), snap.monsters.end(),
                            [](const EntityDTO& e) { return e.type == EntityType::MONSTER; });
    ASSERT_NE(itM, snap.monsters.end());

    // It should still be at 44, 50
    EXPECT_EQ(itM->x, 44);
    EXPECT_EQ(itM->y, 50);
}

// ========================================================================
// 4. TESTS DE SISTEMA DE ÍTEMS (Pick Up & Drop)
// ========================================================================

TEST(WorldTest, World_PickUpItemIntoInventory) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    // Ponemos ítem en el piso donde está el jugador
    mundo.placeItemOnGround(Position{5, 5}, 202, 10);  // 202 es poción roja

    mundo.pickUpItem(1);

    // Verificamos que el mapa ya no tiene el ítem
    auto snap = mundo.generateSnapshot();
    EXPECT_TRUE(snap.groundItems.empty());

    // Y que el jugador lo tiene en su inventario
    // TestWorld no expone el Player interno, pero podemos ver los outgoing events
    auto evs = mundo.pollEvents();
    bool pickedUpEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Objeto recogido.";
    });
    EXPECT_TRUE(pickedUpEvent);
}

TEST(WorldTest, World_PickUpItemNothingToPickUp) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    // No ponemos nada en el piso
    mundo.pickUpItem(1);

    auto evs = mundo.pollEvents();
    bool nothingHereEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "No hay objetos aquí para recoger.";
    });
    EXPECT_TRUE(nothingHereEvent);
}

TEST(WorldTest, World_PickUpItemNoSpaceInInventory) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    // Llenamos el inventario del jugador forzadamente droppeándole ítems hasta que no pueda más?
    // En TestWorld no podemos acceder al player directamente, pero podemos darle 20 items de
    // distinto ID. Como son distinto ID, cada uno ocupará un slot diferente aunque sean
    // stackeables.
    for (int i = 0; i < 20; ++i) {
        mundo.placeItemOnGround(Position{5, 5}, 100 + i, 1);
        mundo.pickUpItem(1);
    }

    // Ahora el inventario debería estar lleno de espadas.
    // Limpiamos los eventos salientes
    mundo.pollEvents();

    // Intentamos agarrar algo nuevo
    mundo.placeItemOnGround(Position{5, 5}, 202, 10);  // pocion roja
    mundo.pickUpItem(1);

    // Verificamos que el item sigo en el piso!
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

TEST(WorldTest, World_DropItemSuccess) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    // Agarra un ítem
    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);

    // Ahora lo tira (slot 0, amount 5)
    mundo.dropItem(1, 0, 5);

    auto snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.groundItems.size(), 1u);
    EXPECT_EQ(snap.groundItems[0].itemId, 202u);
    EXPECT_EQ(snap.groundItems[0].amount, 5);
}

TEST(WorldTest, World_DropItemNoSpaceOnGround) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    // El jugador agarra 1 item
    mundo.placeItemOnGround(Position{5, 5}, 202, 10);
    mundo.pickUpItem(1);

    // Ahora llenamos el piso alrededor del jugador (pos 5,5 y los 8 alrededor)
    mundo.placeItemOnGround(Position{5, 5}, 101, 1);
    mundo.placeItemOnGround(Position{5, 4}, 101, 1);
    mundo.placeItemOnGround(Position{5, 6}, 101, 1);
    mundo.placeItemOnGround(Position{4, 5}, 101, 1);
    mundo.placeItemOnGround(Position{6, 5}, 101, 1);
    mundo.placeItemOnGround(Position{4, 4}, 101, 1);
    mundo.placeItemOnGround(Position{6, 6}, 101, 1);
    mundo.placeItemOnGround(Position{4, 6}, 101, 1);
    mundo.placeItemOnGround(Position{6, 4}, 101, 1);

    mundo.pollEvents();  // Limpiar eventos

    // Intenta tirar
    mundo.dropItem(1, 0, 5);

    auto evs = mundo.pollEvents();
    bool noSpaceEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 &&
               ev.message == "No hay suficiente espacio en el suelo para tirar el objeto.";
    });
    EXPECT_TRUE(noSpaceEvent);
}

TEST(WorldTest, World_PlayerDeathDropsInventoryItems) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(5, 5)));

    // Le damos ítems a Player 1 (3 items distintos en slots diferentes)
    mundo.placeItemOnGround(Position{5, 5}, 202, 10);  // pocion
    mundo.pickUpItem(1);
    mundo.placeItemOnGround(Position{5, 5}, 101, 1);  // espada
    mundo.pickUpItem(1);
    mundo.placeItemOnGround(Position{5, 5}, 102, 1);  // escudo
    mundo.pickUpItem(1);

    // Matamos al Player 1 usando el método directo de la lógica (simulando muerte in-game)
    mundo.handlePlayerDeath(1);

    // El jugador 1 debería estar muerto. Su inventario se tendría que haber caído al piso.
    auto snap = mundo.generateSnapshot();

    // Verificamos que hay por lo menos 3 items en el piso (los que agarró)
    EXPECT_GE(snap.groundItems.size(), 3u);
}

// ========================================================================
// 5. TESTS DE LINEA DE VISION EN COMBATE
// ========================================================================

TEST(WorldTest, World_PlayerCannotAttackThroughObstacle_Straight) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(5, 5)));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, "zone", 0, 0};  // Rango suficiente
    uint32_t monsterId = mundo.addMonster(NPCType::GOBLIN, Position{9, 5}, mConfig);

    // Obstáculo en medio de la línea recta (7, 5)
    mundo.setObstacleAt(7, 5);

    // Player1 ataca al monster
    mundo.playerAttack(1, monsterId);

    auto evs = mundo.pollEvents();
    bool blockedEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Hay un obstaculo bloqueando tu vision.";
    });
    EXPECT_TRUE(blockedEvent);
}

TEST(WorldTest, World_PlayerCannotAttackThroughObstacle_Diagonal) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(5, 5)));

    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, "zone", 0, 0};  // Rango suficiente
    uint32_t monsterId = mundo.addMonster(NPCType::GOBLIN, Position{9, 9}, mConfig);

    // Obstáculo en medio de la línea diagonal (7, 7)
    mundo.setObstacleAt(7, 7);

    // Player1 ataca al monster
    mundo.playerAttack(1, monsterId);

    auto evs = mundo.pollEvents();
    bool blockedEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Hay un obstaculo bloqueando tu vision.";
    });
    EXPECT_TRUE(blockedEvent);
}

TEST(WorldTest, World_PlayerDeathDropsExcessGold) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string p1 = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, p1, makeSpawnData(5, 5)));

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
    EXPECT_EQ(player->getGold(), 5000);  // Le quedo el safe limit
}

TEST(WorldTest, World_PickUpGoldAddsToWallet) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));

    Player* player = mundo.getPlayerById(1);
    ASSERT_NE(player, nullptr);
    uint32_t initialGold = player->getGold();

    // Ponemos oro en el piso
    mundo.placeItemOnGround(Position{5, 5}, 1, 1500);  // 1 = GOLD_ITEM_ID

    mundo.pickUpItem(1);

    EXPECT_EQ(player->getGold(), initialGold + 1500);

    auto evs = mundo.pollEvents();
    bool pickedUpEvent = std::any_of(evs.begin(), evs.end(), [](const auto& ev) {
        return ev.targetDbId == 1 && ev.message == "Recogiste 1500 monedas de oro.";
    });
    EXPECT_TRUE(pickedUpEvent);
}

TEST(WorldTest, World_MonsterDropsLootOnDeath_AndCleanup) {
    ItemRegistry registry("../config/items.toml");
    CharacterConfigs configs = getTestConfigs();
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user, makeSpawnData(5, 5)));
    mundo.setFairPlayRules(false);

    Player* p = mundo.getPlayerById(1);
    p->applyBoost(BoostType::STRENGTH, 100, 10000);  // Para matarlo de 1 golpe

    // Equipar un arma para que el combate ocurra
    Weapon testSword(999, "Espada", 100, WeaponType::MELEE, 50, 100, 2, 0);
    p->equipWeapon(&testSword);

    bool droppedSomething = false;
    for (int i = 0; i < 500; i++) {
        MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 10, 10, "zone", 0, 0};
        uint32_t mId = mundo.addMonster(NPCType::GOBLIN, Position{5, 6}, mConfig);

        mundo.playerAttack(1, mId);
        mundo.update(33.0f);  // Cleanup

        auto snap = mundo.generateSnapshot();
        EXPECT_TRUE(snap.monsters.empty());  // El monstruo debe desaparecer

        if (!snap.groundItems.empty()) {
            droppedSomething = true;
            break;  // Si dropeo algo terminamos temprano
        }
    }

    // Como tiene ~10% de chance, en 500 intentos DEBE dropear algo.
    EXPECT_TRUE(droppedSomething);
}
