#include <gtest/gtest.h>
#include <memory>
#include "World.h"
#include "model/entities/Player.h"

TEST(WorldTest, World_InitializesCorrectly) {
    World mundo(42, "PaladinGM");

    EXPECT_EQ(mundo.getWorldId(), 42);
    EXPECT_EQ(mundo.getCreatorPlayerName(), "PaladinGM");
    EXPECT_TRUE(mundo.isEmpty());
    EXPECT_EQ(mundo.getPlayerCount(), 0);
}

TEST(WorldTest, World_HandlesPlayerLifecycleWithUniquePtr) {
    World mundo(1, "ServerAdmin");

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
    EXPECT_EQ(mundo.getPlayerCount(), 2); // Sigue teniendo 2 originales

    // Remoción y limpieza del mapa
    EXPECT_TRUE(mundo.removePlayer(id1));
    EXPECT_EQ(mundo.getPlayerCount(), 1);

    EXPECT_TRUE(mundo.removePlayer(id2));
    EXPECT_TRUE(mundo.isEmpty());
}

TEST(WorldTest, World_RemoveNonExistentPlayerReturnsFalse) {
    World mundo(1, "Tester");
    
    // Intentar sacar a alguien de un mundo vacío no debería romper nada
    EXPECT_FALSE(mundo.removePlayer(999));
}

TEST(WorldTest, World_GenerateSnapshotWithPlayersCorrectly) {
    // 1. Inicializamos un mundo vacío
    World mundo(42, "PaladinGM");

    // Verificamos que el snapshot inicial esté vacío
    SnapshotDTO snapshotInicial = mundo.generateSnapshot();
    EXPECT_TRUE(snapshotInicial.entities.empty());

    // 2. Simulamos el login de dos jugadores (gatilla la creación de PlayerMocks)
    std::string user1 = "Aoki";
    std::string user2 = "Beren";
    
    ASSERT_TRUE(mundo.addPlayer(100, user1));
    ASSERT_TRUE(mundo.addPlayer(200, user2));

    // 3. Modificamos la posición de uno para testear que el snapshot arrastre datos vivos
    // (Simula un comando de movimiento previo al snapshot)
    mundo.moveEntity(100, static_cast<uint8_t>(Movement::DOWN)); // y: 0 -> 1
    mundo.moveEntity(100, static_cast<uint8_t>(Movement::RIGHT)); // x: 0 -> 1

    // 4. Generamos el Snapshot que se le enviaría al cliente
    SnapshotDTO snapshotActual = mundo.generateSnapshot();

    // 5. Validaciones de la estructura del SnapshotDTO
    ASSERT_EQ(snapshotActual.entities.size(), 2);

    bool encontroPlayer1 = false;
    bool encontroPlayer2 = false;
    int spritesEvaluados = 0;

    for (const auto& entity : snapshotActual.entities) {
        spritesEvaluados++; // El primero que salga se lleva el 1, el segundo el 2

        if (entity.id == 100) {
            encontroPlayer1 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 1);
            EXPECT_EQ(entity.y, 1);
            EXPECT_EQ(entity.current_hp, 100);
            EXPECT_EQ(entity.max_hp, 100);
            
            // Validamos que su sprite coincida con el orden de salida real en el loop
            EXPECT_EQ(entity.sprite_id, spritesEvaluados);
        } 
        else if (entity.id == 200) {
            encontroPlayer2 = true;
            EXPECT_EQ(entity.type, EntityType::PLAYER);
            EXPECT_EQ(entity.x, 0);
            EXPECT_EQ(entity.y, 0);
            EXPECT_EQ(entity.current_hp, 100);
            EXPECT_EQ(entity.max_hp, 100);
            
            // Validamos que su sprite coincida con el orden de salida real en el loop
            EXPECT_EQ(entity.sprite_id, spritesEvaluados);
        }
    }

    EXPECT_TRUE(encontroPlayer1);
    EXPECT_TRUE(encontroPlayer2);
}
