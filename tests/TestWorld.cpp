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