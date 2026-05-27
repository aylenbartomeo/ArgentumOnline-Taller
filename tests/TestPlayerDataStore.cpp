#include <filesystem>

#include <gtest/gtest.h>

#include "../server/src/persistence/PlayerDataStore.h"

namespace fs = std::filesystem;

class PlayerDataStoreTest: public ::testing::Test {
protected:
    std::string testDir = "test_game_data/";

    void SetUp() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }
};

TEST_F(PlayerDataStoreTest, LoadReturnsNulloptForUnknownPlayer) {
    PlayerDataStore store(testDir);
    auto data = store.loadPlayerData("UnknownPlayer");
    EXPECT_FALSE(data.has_value());
}

TEST_F(PlayerDataStoreTest, SaveAndLoadRoundTrip) {
    PlayerDataStore store(testDir);

    PlayerPersistData dataToSave{};
    dataToSave.dbId = 123;
    dataToSave.posX = 5;
    dataToSave.posY = 3;

    store.savePlayerData("Franco", dataToSave);

    auto loadedData = store.loadPlayerData("Franco");
    ASSERT_TRUE(loadedData.has_value());
    EXPECT_EQ(loadedData->dbId, 123);
    EXPECT_EQ(loadedData->posX, 5);
    EXPECT_EQ(loadedData->posY, 3);
}

TEST_F(PlayerDataStoreTest, SaveOverwritesExistingData) {
    PlayerDataStore store(testDir);

    PlayerPersistData initialData{};
    initialData.dbId = 42;
    initialData.posX = 10;
    initialData.posY = 10;
    store.savePlayerData("Cami", initialData);

    PlayerPersistData updatedData{};
    updatedData.dbId = 42;
    updatedData.posX = 20;
    updatedData.posY = 20;
    store.savePlayerData("Cami", updatedData);

    auto loadedData = store.loadPlayerData("Cami");
    ASSERT_TRUE(loadedData.has_value());
    EXPECT_EQ(loadedData->dbId, 42);
    EXPECT_EQ(loadedData->posX, 20);
    EXPECT_EQ(loadedData->posY, 20);
}

TEST_F(PlayerDataStoreTest, MultiplePlayersPersistIndependently) {
    PlayerDataStore store(testDir);

    PlayerPersistData p1{};
    p1.dbId = 1;
    p1.posX = 1;
    p1.posY = 1;

    PlayerPersistData p2{};
    p2.dbId = 2;
    p2.posX = 2;
    p2.posY = 2;

    store.savePlayerData("Player1", p1);
    store.savePlayerData("Player2", p2);

    auto load1 = store.loadPlayerData("Player1");
    auto load2 = store.loadPlayerData("Player2");

    ASSERT_TRUE(load1.has_value());
    EXPECT_EQ(load1->dbId, 1);

    ASSERT_TRUE(load2.has_value());
    EXPECT_EQ(load2->dbId, 2);
}

TEST_F(PlayerDataStoreTest, IndexSurvivesRestart) {
    {
        PlayerDataStore store(testDir);
        PlayerPersistData data{};
        data.dbId = 99;
        data.posX = 7;
        data.posY = 7;
        store.savePlayerData("PersistentPlayer", data);
    }

    // Simulate restart by creating a new store instance pointing to the same directory
    {
        PlayerDataStore newStore(testDir);
        auto loaded = newStore.loadPlayerData("PersistentPlayer");
        ASSERT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->dbId, 99);
        EXPECT_EQ(loaded->posX, 7);
        EXPECT_EQ(loaded->posY, 7);
    }
}
