#include <fstream>
#include <memory>

#include <gtest/gtest.h>

#include "model/entities/Player.h"

#include "World.h"
#include "model/items/ItemRegistry.h"

TEST(WorldTest, World_InitializesCorrectly) {
    ItemRegistry registry("../config/items.toml");
    World mundo(42, "PaladinGM", registry);

    EXPECT_EQ(mundo.getWorldId(), 42);
    EXPECT_EQ(mundo.getCreatorPlayerName(), "PaladinGM");
    EXPECT_TRUE(mundo.isEmpty());
    EXPECT_EQ(mundo.getPlayerCount(), 0);
}

TEST(WorldTest, World_HandlesPlayerLifecycleWithUniquePtr) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "ServerAdmin", registry);

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
    World mundo(1, "Tester", registry);
    std::string user = "EdgeWalker";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    mundo.moveEntity(1, Movement::UP);
    mundo.moveEntity(1, Movement::LEFT);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    EXPECT_EQ(snap.entities[0].x, 0);
    EXPECT_EQ(snap.entities[0].y, 0);
}

TEST(WorldTest, World_RemoveNonExistentPlayerReturnsFalse) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);

    // Intentar sacar a alguien de un mundo vacío no debería romper nada
    EXPECT_FALSE(mundo.removePlayer(999));
}

TEST(WorldTest, World_GenerateSnapshotWithPlayersCorrectly) {
    // 1. Inicializamos un mundo vacío
    ItemRegistry registry("../config/items.toml");
    World mundo(42, "PaladinGM", registry);

    // Verificamos que el snapshot inicial esté vacío
    SnapshotDTO snapshotInicial = mundo.generateSnapshot();
    EXPECT_TRUE(snapshotInicial.entities.empty());

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
    ASSERT_EQ(snapshotActual.entities.size(), 2);

    bool encontroPlayer1 = false;
    bool encontroPlayer2 = false;
    int spritesEvaluados = 0;

    for (const auto& entity: snapshotActual.entities) {
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
    World mundo(1, "Tester", registry);
    std::string user = "Blocker";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // El jugador empieza en (0,0). Ponemos obstáculo en (1,0).
    mundo.setObstacleAt(1, 0);

    mundo.moveEntity(1, Movement::RIGHT);  // Intentar ir a (1,0) - bloqueado

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    EXPECT_EQ(snap.entities[0].x, 0);  // No se movió
    EXPECT_EQ(snap.entities[0].y, 0);
}

TEST(WorldTest, World_UpdateTriggersMonsterAttack) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // Config de monstruo con rango de ataque suficiente
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 2, 1, "zone"};
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
        if (snap.entities[0].current_hp < 15) {
            tookDamage = true;
            break;
        }
    }

    EXPECT_TRUE(tookDamage);
}

TEST(WorldTest, World_UpdateDoesNotTriggerMonsterAttackIfOutOfRange) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    std::string user = "Player1";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    // Config de monstruo con rango de ataque pequeño
    MonsterConfig mConfig = {10, 5, 0, 10, 20, 5, 1, 1, "zone"};
    Position mPos = {3, 0};  // Player está en {0, 0}, dist = 3.
    mundo.addMonster(NPCType::GOBLIN, mPos, mConfig);

    mundo.update(1.0f);

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    EXPECT_EQ(snap.entities[0].current_hp, 15);  // NO recibió daño
}

TEST(WorldTest, World_AddPlayerSpawnsAtMapSpawn) {
    const std::string path = "/tmp/test_world_spawn.json";
    std::ofstream out(path);
    out << R"({"tileSize":16,"tileset":"x.png","tilesetCols":12,"width":20,"height":15,)"
        << R"("spawn":{"x":3,"y":4},"tiles":[]})";
    out.close();

    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    ASSERT_TRUE(mundo.loadMap(path));

    std::string user = "Spawner";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    EXPECT_EQ(snap.entities[0].x, 3);
    EXPECT_EQ(snap.entities[0].y, 4);
}

TEST(WorldTest, World_AddPlayerWithSavedPositionSpawnsThere) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    
    std::string user = "SavedPlayer";
    Position savedPos{5, 5};
    ASSERT_TRUE(mundo.addPlayer(1, user, savedPos));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    EXPECT_EQ(snap.entities[0].x, 5);
    EXPECT_EQ(snap.entities[0].y, 5);
}

TEST(WorldTest, World_AddPlayerWithInvalidPositionUsesDefault) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    
    std::string user = "InvalidPosPlayer";
    Position invalidPos{-1, -1}; // assuming this is out of bounds
    ASSERT_TRUE(mundo.addPlayer(1, user, invalidPos));

    SnapshotDTO snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.entities.size(), 1u);
    auto defaultPos = mundo.getInitialPosition();
    EXPECT_EQ(snap.entities[0].x, defaultPos.first);
    EXPECT_EQ(snap.entities[0].y, defaultPos.second);
}

TEST(WorldTest, World_GetPlayerPositionReturnsCurrentPos) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    
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
    World mundo(1, "Tester", registry);
    
    std::string user = "TestUser";
    ASSERT_TRUE(mundo.addPlayer(42, user));
    
    auto name = mundo.getPlayerUsername(42);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "TestUser");
}

TEST(WorldTest, World_GetOnlinePlayerDbIdsReturnsAllActive) {
    ItemRegistry registry("../config/items.toml");
    World mundo(1, "Tester", registry);
    
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
    for (auto id : expected) {
        EXPECT_NE(std::find(ids.begin(), ids.end(), id), ids.end());
    }
}
