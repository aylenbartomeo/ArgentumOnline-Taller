/**
 * Verifica el flujo completo de guardado y carga de estado de jugador:
 *   1. PlayerDataStore  — serialización binaria de todos los campos nuevos
 *   2. StatsComponent   — getters getRace() / getCharacterClass()
 *   3. World            — round-trip addPlayer → getPlayerPersistData → addPlayer
 */

#include <filesystem>
#include <optional>

#include <gtest/gtest.h>

#include "config/CharacterConfig.h"
#include "model/components/StatsComponent.h"
#include "model/items/ItemRegistry.h"
#include "persistence/PlayerDataStore.h"

#include "World.h"

namespace fs = std::filesystem;

// ============================================================================
// Helpers compartidos
// ============================================================================

// CharacterConfigs mínimos para construir un World en tests
static CharacterConfigs makeTestConfigs() {
    RaceConfig human{1.0f, 1.0f, 1.0f};
    RaceConfig elf{0.8f, 1.3f, 1.1f};
    RaceConfig dwarf{1.3f, 0.7f, 0.9f};
    RaceConfig gnome{1.1f, 1.2f, 1.0f};

    CharacterClassConfig mage{0.7f, 1.5f, 1.5f, true};
    CharacterClassConfig warrior{1.4f, 0.0f, 0.0f, false};
    CharacterClassConfig paladin{1.2f, 0.7f, 0.6f, true};
    CharacterClassConfig cleric{1.0f, 1.1f, 1.1f, true};

    PlayerConfig base{15, 15, 15, 15, 1, 0, 0};

    return CharacterConfigs{
            base,
            {
                    {Race::HUMAN, human},
                    {Race::ELF, elf},
                    {Race::DWARF, dwarf},
                    {Race::GNOME, gnome},
            },
            {
                    {CharacterClass::MAGE, mage},
                    {CharacterClass::WARRIOR, warrior},
                    {CharacterClass::PALADIN, paladin},
                    {CharacterClass::CLERIC, cleric},
            },
    };
}

static InventoryConfig getTestInventoryConfig() { return {16, 0, 10000, 5000}; }

// Construye un PlayerPersistData con todos los campos relevantes completados
static PlayerPersistData makeFullPersistData(uint32_t dbId, int32_t x, int32_t y) {
    PlayerPersistData d{};
    d.dbId = dbId;
    d.posX = x;
    d.posY = y;
    d.hp = 80;
    d.mana = 40;
    d.level = 3;
    d.exp = 500;
    d.gold = 250;
    d.stateId = 0;  // Normal
    d.race = static_cast<uint8_t>(Race::ELF);
    d.characterClass = static_cast<uint8_t>(CharacterClass::MAGE);
    d.inventorySize = 2;
    d.inventory[0] = {101, 3};  // item_id=101, amount=3
    d.inventory[1] = {202, 1};
    return d;
}

// ============================================================================
// 1. PLAYERDATASTORE — campos extendidos
// ============================================================================

class PlayerDataStorePersistenceTest: public ::testing::Test {
protected:
    std::string testDir = "test_persist_data/";

    void SetUp() override {
        fs::remove_all(testDir);
        fs::create_directories(testDir);
    }
    void TearDown() override { fs::remove_all(testDir); }
};

TEST_F(PlayerDataStorePersistenceTest, SaveAndLoad_AllNewFields_RoundTrip) {
    PlayerDataStore store(testDir);
    PlayerPersistData original = makeFullPersistData(1, 5, 7);

    store.savePlayerData("Elara", original);
    auto loaded = store.loadPlayerData("Elara");

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->dbId, original.dbId);
    EXPECT_EQ(loaded->posX, original.posX);
    EXPECT_EQ(loaded->posY, original.posY);
    EXPECT_EQ(loaded->hp, original.hp);
    EXPECT_EQ(loaded->mana, original.mana);
    EXPECT_EQ(loaded->level, original.level);
    EXPECT_EQ(loaded->exp, original.exp);
    EXPECT_EQ(loaded->gold, original.gold);
    EXPECT_EQ(loaded->stateId, original.stateId);
    EXPECT_EQ(loaded->race, original.race);
    EXPECT_EQ(loaded->characterClass, original.characterClass);
}

TEST_F(PlayerDataStorePersistenceTest, SaveAndLoad_Inventory_RoundTrip) {
    PlayerDataStore store(testDir);
    PlayerPersistData original = makeFullPersistData(2, 0, 0);

    store.savePlayerData("Thorin", original);
    auto loaded = store.loadPlayerData("Thorin");

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->inventorySize, 2);
    EXPECT_EQ(loaded->inventory[0].item_id, 101u);
    EXPECT_EQ(loaded->inventory[0].amount, 3);
    EXPECT_EQ(loaded->inventory[1].item_id, 202u);
    EXPECT_EQ(loaded->inventory[1].amount, 1);
}

TEST_F(PlayerDataStorePersistenceTest, Overwrite_UpdatesAllFields) {
    PlayerDataStore store(testDir);

    PlayerPersistData v1 = makeFullPersistData(10, 1, 1);
    store.savePlayerData("Kira", v1);

    PlayerPersistData v2 = makeFullPersistData(10, 9, 9);
    v2.hp = 50;
    v2.mana = 10;
    v2.level = 7;
    v2.exp = 9999;
    v2.gold = 1000;
    v2.race = static_cast<uint8_t>(Race::DWARF);
    v2.characterClass = static_cast<uint8_t>(CharacterClass::WARRIOR);
    v2.inventorySize = 1;
    v2.inventory[0] = {303, 5};
    store.savePlayerData("Kira", v2);

    auto loaded = store.loadPlayerData("Kira");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->posX, 9);
    EXPECT_EQ(loaded->posY, 9);
    EXPECT_EQ(loaded->hp, 50);
    EXPECT_EQ(loaded->level, 7);
    EXPECT_EQ(loaded->exp, 9999u);
    EXPECT_EQ(loaded->gold, 1000u);
    EXPECT_EQ(loaded->race, static_cast<uint8_t>(Race::DWARF));
    EXPECT_EQ(loaded->characterClass, static_cast<uint8_t>(CharacterClass::WARRIOR));
    EXPECT_EQ(loaded->inventory[0].item_id, 303u);
    EXPECT_EQ(loaded->inventory[0].amount, 5);
}

TEST_F(PlayerDataStorePersistenceTest, Overwrite_DoesNotCorruptOtherPlayers) {
    PlayerDataStore store(testDir);

    store.savePlayerData("Alpha", makeFullPersistData(1, 1, 1));
    store.savePlayerData("Beta", makeFullPersistData(2, 2, 2));

    // Sobreescribir solo Alpha
    PlayerPersistData updated = makeFullPersistData(1, 9, 9);
    store.savePlayerData("Alpha", updated);

    auto beta = store.loadPlayerData("Beta");
    ASSERT_TRUE(beta.has_value());
    EXPECT_EQ(beta->posX, 2);
    EXPECT_EQ(beta->posY, 2);
}

TEST_F(PlayerDataStorePersistenceTest, PersistsAcrossStoreRestart_AllFields) {
    {
        PlayerDataStore store(testDir);
        store.savePlayerData("Nyx", makeFullPersistData(99, 3, 4));
    }
    {
        PlayerDataStore store(testDir);
        auto loaded = store.loadPlayerData("Nyx");
        ASSERT_TRUE(loaded.has_value());
        EXPECT_EQ(loaded->dbId, 99u);
        EXPECT_EQ(loaded->hp, 80);
        EXPECT_EQ(loaded->mana, 40);
        EXPECT_EQ(loaded->level, 3);
        EXPECT_EQ(loaded->exp, 500u);
        EXPECT_EQ(loaded->gold, 250u);
        EXPECT_EQ(loaded->race, static_cast<uint8_t>(Race::ELF));
        EXPECT_EQ(loaded->characterClass, static_cast<uint8_t>(CharacterClass::MAGE));
        EXPECT_EQ(loaded->inventory[0].item_id, 101u);
    }
}

TEST_F(PlayerDataStorePersistenceTest, GhostState_IsSavedAndRestored) {
    PlayerDataStore store(testDir);
    PlayerPersistData d = makeFullPersistData(5, 0, 0);
    d.stateId = 1;  // Ghost
    store.savePlayerData("GhostPlayer", d);

    auto loaded = store.loadPlayerData("GhostPlayer");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->stateId, 1u);
}

TEST_F(PlayerDataStorePersistenceTest, MeditatingState_IsSavedAndRestored) {
    PlayerDataStore store(testDir);
    PlayerPersistData d = makeFullPersistData(6, 0, 0);
    d.stateId = 2;  // Meditating
    store.savePlayerData("Monk", d);

    auto loaded = store.loadPlayerData("Monk");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->stateId, 2u);
}

// ============================================================================
// 2. STATSCOMPONENT — getRace() / getCharacterClass()
// ============================================================================

class StatsComponentRaceClassTest: public ::testing::Test {
protected:
    RaceConfig elfRace{0.8f, 1.3f, 1.1f};
    RaceConfig dwarfRace{1.3f, 0.7f, 0.9f};
    CharacterClassConfig mageClass{0.7f, 1.5f, 1.5f, true};
    CharacterClassConfig warriorClass{1.4f, 0.0f, 0.0f, false};
    PlayerConfig base{15, 15, 15, 15, 1, 0, 0};
};

TEST_F(StatsComponentRaceClassTest, GetRace_ReturnsConstructedEnum) {
    StatsComponent stats(Race::ELF, CharacterClass::MAGE, elfRace, mageClass, base);
    EXPECT_EQ(stats.getRace(), Race::ELF);
}

TEST_F(StatsComponentRaceClassTest, GetCharacterClass_ReturnsConstructedEnum) {
    StatsComponent stats(Race::ELF, CharacterClass::MAGE, elfRace, mageClass, base);
    EXPECT_EQ(stats.getCharacterClass(), CharacterClass::MAGE);
}

TEST_F(StatsComponentRaceClassTest, GetRace_Dwarf) {
    StatsComponent stats(Race::DWARF, CharacterClass::WARRIOR, dwarfRace, warriorClass, base);
    EXPECT_EQ(stats.getRace(), Race::DWARF);
}

TEST_F(StatsComponentRaceClassTest, GetCharacterClass_Warrior) {
    StatsComponent stats(Race::DWARF, CharacterClass::WARRIOR, dwarfRace, warriorClass, base);
    EXPECT_EQ(stats.getCharacterClass(), CharacterClass::WARRIOR);
}

TEST_F(StatsComponentRaceClassTest, AllRaces_StoreCorrectly) {
    RaceConfig human{1.0f, 1.0f, 1.0f};
    RaceConfig gnome{1.1f, 1.2f, 1.0f};

    StatsComponent s1(Race::HUMAN, CharacterClass::CLERIC, human, {1.0f, 1.1f, 1.1f, true}, base);
    StatsComponent s2(Race::GNOME, CharacterClass::PALADIN, gnome, {1.2f, 0.7f, 0.6f, true}, base);

    EXPECT_EQ(s1.getRace(), Race::HUMAN);
    EXPECT_EQ(s2.getRace(), Race::GNOME);
    EXPECT_EQ(s1.getCharacterClass(), CharacterClass::CLERIC);
    EXPECT_EQ(s2.getCharacterClass(), CharacterClass::PALADIN);
}

// ============================================================================
// 3. WORLD — getPlayerPersistData() y round-trip completo
// ============================================================================

class WorldPersistenceTest: public ::testing::Test {
protected:
    ItemRegistry registry{"../config/items.toml"};
    CharacterConfigs configs = makeTestConfigs();
};

// --- getPlayerPersistData ---

TEST_F(WorldPersistenceTest, GetPlayerPersistData_ReturnsNulloptForUnknownId) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    EXPECT_FALSE(mundo.getPlayerPersistData(999).has_value());
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_ReturnsData_AfterAddPlayer) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Arden";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->dbId, 1u);
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_ReturnsNullopt_AfterRemovePlayer) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Arden";
    ASSERT_TRUE(mundo.addPlayer(1, user));
    mundo.removePlayer(1);

    EXPECT_FALSE(mundo.getPlayerPersistData(1).has_value());
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_Position_MatchesCurrentPosition) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Mover";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    mundo.moveEntity(1, Movement::RIGHT);
    mundo.moveEntity(1, Movement::DOWN);

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->posX, 1);
    EXPECT_EQ(data->posY, 1);
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_Race_MatchesSavedRace) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Elara";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.race = static_cast<uint8_t>(Race::ELF);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->race, static_cast<uint8_t>(Race::ELF));
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_Class_MatchesSavedClass) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Archmage";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.characterClass = static_cast<uint8_t>(CharacterClass::MAGE);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->characterClass, static_cast<uint8_t>(CharacterClass::MAGE));
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_Level_MatchesSavedLevel) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Veteran";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.level = 5;
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->level, 5);
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_Gold_MatchesSavedGold) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "RichPlayer";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.gold = 9999;
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->gold, 9999u);
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_GhostState_IsPersisted) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "DeadPlayer";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.stateId = 1;  // Ghost
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->stateId, 1u);
}

TEST_F(WorldPersistenceTest, GetPlayerPersistData_MeditatingState_IsPersisted) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Monk";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.stateId = 2;  // Meditating
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->stateId, 2u);
}

// --- Round-trip completo (simula disconnect → reconnect) ---

TEST_F(WorldPersistenceTest, RoundTrip_NewPlayer_DefaultsAreConsistent) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Newbie";
    ASSERT_TRUE(mundo.addPlayer(1, user));

    auto data = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->level, 1);
    EXPECT_EQ(data->exp, 0u);
    EXPECT_GT(data->hp, 0);
    EXPECT_GE(data->mana, 0);
}

TEST_F(WorldPersistenceTest, RoundTrip_Position_IsRestoredAfterReconnect) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Traveler";

    // Primera sesión: añadir y mover
    PlayerPersistData firstSeed = makeFullPersistData(1, 0, 0);
    ASSERT_TRUE(mundo.addPlayer(1, user, firstSeed));
    mundo.moveEntity(1, Movement::RIGHT);
    mundo.moveEntity(1, Movement::RIGHT);

    // Guardar y desconectar
    auto saved = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(saved.has_value());
    mundo.removePlayer(1);

    // Segunda sesión: reconectar con datos guardados
    ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

    auto snap = mundo.generateSnapshot();
    ASSERT_EQ(snap.players.size(), 1u);
    EXPECT_EQ(snap.players[0].x, saved->posX);
    EXPECT_EQ(snap.players[0].y, saved->posY);
}

TEST_F(WorldPersistenceTest, RoundTrip_RaceAndClass_SurviveReconnect) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Elven Mage";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.race = static_cast<uint8_t>(Race::ELF);
    seed.characterClass = static_cast<uint8_t>(CharacterClass::MAGE);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto saved = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(saved.has_value());
    mundo.removePlayer(1);

    ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

    auto after = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(after.has_value());
    EXPECT_EQ(after->race, static_cast<uint8_t>(Race::ELF));
    EXPECT_EQ(after->characterClass, static_cast<uint8_t>(CharacterClass::MAGE));
}

TEST_F(WorldPersistenceTest, RoundTrip_Level_SurvivesReconnect) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Veteran";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.level = 5;
    seed.exp = 4000;
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto saved = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(saved.has_value());
    mundo.removePlayer(1);

    ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

    auto after = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(after.has_value());
    EXPECT_EQ(after->level, 5);
    EXPECT_EQ(after->exp, 4000u);
}

TEST_F(WorldPersistenceTest, RoundTrip_Equipment_SurvivesReconnect) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Knight";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.inventorySize = 1;
    seed.inventory[0] = {1000, 1};
    seed.equippedSlots = (1u << 0);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto saved = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(saved.has_value());
    EXPECT_TRUE(saved->equippedSlots & (1u << 0));

    mundo.removePlayer(1);
    ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

    auto after = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(after.has_value());
    EXPECT_TRUE(after->equippedSlots & (1u << 0));
}

TEST_F(WorldPersistenceTest, RoundTrip_EquipmentWithGap_SurvivesReconnect) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
    std::string user = "Mage";

    PlayerPersistData seed = makeFullPersistData(1, 0, 0);
    seed.inventorySize = 3;
    seed.inventory[0] = {1000, 1};
    seed.inventory[1] = {0, 0};
    seed.inventory[2] = {1010, 1};
    seed.equippedSlots = (1u << 2);
    ASSERT_TRUE(mundo.addPlayer(1, user, seed));

    auto saved = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(saved.has_value());
    EXPECT_EQ(saved->inventory[2].item_id, 1010u);
    EXPECT_TRUE(saved->equippedSlots & (1u << 2));

    mundo.removePlayer(1);
    ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

    auto after = mundo.getPlayerPersistData(1);
    ASSERT_TRUE(after.has_value());
    EXPECT_EQ(after->inventory[2].item_id, 1010u);
    EXPECT_TRUE(after->equippedSlots & (1u << 2));
}

// TEST_F(WorldPersistenceTest, RoundTrip_Inventory_SurvivesReconnect) {
//     World mundo(1, "Tester", registry, configs, getTestInventoryConfig());
//     std::string user = "Collector";

//     PlayerPersistData seed = makeFullPersistData(1, 0, 0);
//     // Items con IDs que deben existir en items.toml; ajustar si es necesario
//     seed.inventorySize  = 2;
//     seed.inventory[0]   = {1, 5};
//     seed.inventory[1]   = {2, 1};
//     ASSERT_TRUE(mundo.addPlayer(1, user, seed));

//     auto saved = mundo.getPlayerPersistData(1);
//     ASSERT_TRUE(saved.has_value());
//     mundo.removePlayer(1);

//     ASSERT_TRUE(mundo.addPlayer(1, user, saved.value()));

//     auto after = mundo.getPlayerPersistData(1);
//     ASSERT_TRUE(after.has_value());
//     // Verificar que el inventario restaurado contiene los ítems
//     bool foundItem1 = false;
//     bool foundItem2 = false;
//     for (uint8_t i = 0; i < after->inventorySize; ++i) {
//         if (after->inventory[i].item_id == 1 && after->inventory[i].amount == 5) foundItem1 =
//         true; if (after->inventory[i].item_id == 2 && after->inventory[i].amount == 1) foundItem2
//         = true;
//     }
//     EXPECT_TRUE(foundItem1);
//     EXPECT_TRUE(foundItem2);
// }

TEST_F(WorldPersistenceTest, RoundTrip_MultipleAllRaces_AllClasses) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    struct Case {
        uint32_t id;
        Race race;
        CharacterClass cls;
    };
    std::vector<Case> cases = {
            {1, Race::HUMAN, CharacterClass::WARRIOR},
            {2, Race::ELF, CharacterClass::MAGE},
            {3, Race::DWARF, CharacterClass::PALADIN},
            {4, Race::GNOME, CharacterClass::CLERIC},
    };

    for (auto& c: cases) {
        std::string user = "Player" + std::to_string(c.id);
        PlayerPersistData seed = makeFullPersistData(c.id, 0, 0);
        seed.race = static_cast<uint8_t>(c.race);
        seed.characterClass = static_cast<uint8_t>(c.cls);
        ASSERT_TRUE(mundo.addPlayer(c.id, user, seed));
    }

    for (auto& c: cases) {
        auto data = mundo.getPlayerPersistData(c.id);
        ASSERT_TRUE(data.has_value()) << "Missing data for id=" << c.id;
        EXPECT_EQ(data->race, static_cast<uint8_t>(c.race));
        EXPECT_EQ(data->characterClass, static_cast<uint8_t>(c.cls));
    }
}

// ============================================================================
// 4. WORLDDATASTORE — Clanes, Banco, Monstruos, Ítems
// ============================================================================

#include <cstring>

#include "persistence/WorldDataStore.h"

class WorldDataStorePersistenceTest: public ::testing::Test {
protected:
    std::string testDir = "test_world_data/";

    void SetUp() override {
        fs::remove_all(testDir);
        fs::create_directories(testDir);
    }
    void TearDown() override { fs::remove_all(testDir); }
};

TEST_F(WorldDataStorePersistenceTest, SaveAndLoad_Monsters) {
    WorldDataStore store(testDir);
    uint32_t worldId = store.createWorld("TestWorld", "maps/test.json");

    std::vector<MonsterPersistData> monsters;
    MonsterPersistData m{};
    m.entityId = 100;
    m.type = 2;
    m.posX = 10;
    m.posY = 20;
    m.hp = 50;
    m.maxHp = 100;
    monsters.push_back(m);

    store.saveMonsters(worldId, monsters);

    auto loaded = store.loadMonsters(worldId);
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].entityId, 100u);
    EXPECT_EQ(loaded[0].hp, 50);
}

TEST_F(WorldDataStorePersistenceTest, SaveAndLoad_GroundItems) {
    WorldDataStore store(testDir);
    uint32_t worldId = store.createWorld("TestWorld", "maps/test.json");

    std::vector<GroundItemPersistData> items;
    GroundItemPersistData i{};
    i.posX = 5;
    i.posY = 5;
    i.itemId = 1001;
    i.amount = 3;
    items.push_back(i);

    store.saveGroundItems(worldId, items);

    auto loaded = store.loadGroundItems(worldId);
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].itemId, 1001u);
    EXPECT_EQ(loaded[0].amount, 3);
}

TEST_F(WorldDataStorePersistenceTest, SaveAndLoad_Clans) {
    WorldDataStore store(testDir);
    uint32_t worldId = store.createWorld("TestWorld", "maps/test.json");

    std::vector<ClanHeaderPersistData> headers;
    std::vector<std::vector<ClanPlayerPersistData>> members, pending, banned;

    ClanHeaderPersistData h{};
    h.clanId = 1;
    h.founderDbId = 10;
    std::snprintf(h.name, sizeof(h.name), "LosPibes");
    h.memberCount = 2;
    h.pendingCount = 1;
    h.bannedCount = 1;
    headers.push_back(h);

    members.push_back({{10}, {20}});
    pending.push_back({{30}});
    banned.push_back({{40}});

    store.saveClans(worldId, headers, members, pending, banned);

    auto [lHeaders, lMembers, lPending, lBanned] = store.loadClans(worldId);
    ASSERT_EQ(lHeaders.size(), 1u);
    EXPECT_EQ(lHeaders[0].clanId, 1u);
    EXPECT_STREQ(lHeaders[0].name, "LosPibes");
    EXPECT_EQ(lHeaders[0].memberCount, 2u);

    ASSERT_EQ(lMembers.size(), 1u);
    EXPECT_EQ(lMembers[0].size(), 2u);
    EXPECT_EQ(lMembers[0][0].dbId, 10u);
    EXPECT_EQ(lMembers[0][1].dbId, 20u);

    ASSERT_EQ(lPending.size(), 1u);
    EXPECT_EQ(lPending[0][0].dbId, 30u);

    ASSERT_EQ(lBanned.size(), 1u);
    EXPECT_EQ(lBanned[0][0].dbId, 40u);
}

TEST_F(WorldDataStorePersistenceTest, SaveAndLoad_BankAccounts) {
    WorldDataStore store(testDir);
    uint32_t worldId = store.createWorld("TestWorld", "maps/test.json");

    std::vector<BankAccountHeaderPersistData> headers;
    std::vector<std::vector<BankSlotPersistData>> slots;

    BankAccountHeaderPersistData h{};
    h.playerDbId = 99;
    h.gold = 5000;
    h.slotCount = 2;
    headers.push_back(h);

    slots.push_back({{1001, 5, {}}, {2001, 1, {}}});

    store.saveBankAccounts(worldId, headers, slots);

    auto [lHeaders, lSlots] = store.loadBankAccounts(worldId);
    ASSERT_EQ(lHeaders.size(), 1u);
    EXPECT_EQ(lHeaders[0].playerDbId, 99u);
    EXPECT_EQ(lHeaders[0].gold, 5000u);

    ASSERT_EQ(lSlots.size(), 1u);
    EXPECT_EQ(lSlots[0].size(), 2u);
    EXPECT_EQ(lSlots[0][0].itemId, 1001u);
    EXPECT_EQ(lSlots[0][0].amount, 5);
}

// ============================================================================
// 5. WORLD — Persistence Integration (Clanes, Bank)
// ============================================================================

TEST_F(WorldPersistenceTest, World_BankPersistence_RoundTrip) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Restaurar manualmente una cuenta bancaria
    std::vector<BankAccountHeaderPersistData> headers;
    std::vector<std::vector<BankSlotPersistData>> slots;

    BankAccountHeaderPersistData h{};
    h.playerDbId = 5;
    h.gold = 1000;
    h.slotCount = 1;
    headers.push_back(h);
    slots.push_back({{1001, 10, {}}});

    mundo.restoreBank({headers, slots});

    // Ahora leemos la data de vuelta
    auto bankData = mundo.getBankPersistData();
    auto outHeaders = bankData.headers;
    auto outSlots = bankData.slots;

    ASSERT_EQ(outHeaders.size(), 1u);
    EXPECT_EQ(outHeaders[0].playerDbId, 5u);
    EXPECT_EQ(outHeaders[0].gold, 1000u);
    EXPECT_EQ(outHeaders[0].slotCount, 1u);

    ASSERT_EQ(outSlots.size(), 1u);
    EXPECT_EQ(outSlots[0][0].itemId, 1001u);
    EXPECT_EQ(outSlots[0][0].amount, 10u);
}

TEST_F(WorldPersistenceTest, World_ClanPersistence_RoundTrip) {
    World mundo(1, "Tester", registry, configs, getTestInventoryConfig());

    // Restaurar manualmente un clan
    std::vector<ClanHeaderPersistData> headers;
    std::vector<std::vector<ClanPlayerPersistData>> members, pending, banned;

    ClanHeaderPersistData h{};
    h.clanId = 1;
    h.founderDbId = 1;
    std::snprintf(h.name, sizeof(h.name), "Imperio");
    h.memberCount = 2;
    h.pendingCount = 0;
    h.bannedCount = 0;
    headers.push_back(h);
    members.push_back({{1}, {2}});
    pending.push_back({});
    banned.push_back({});

    mundo.restoreClans({headers, members, pending, banned});

    // Verificamos que areClanmates funcione sin tener a los jugadores logueados
    // Pero areClanmates chequea también getClanIdOfPlayer internamente
    EXPECT_TRUE(mundo.areClanmates(1, 2));

    // Y recuperamos para comprobar
    auto clanData = mundo.getClansPersistData();
    auto outH = clanData.headers;
    auto outM = clanData.members;

    ASSERT_EQ(outH.size(), 1u);
    EXPECT_STREQ(outH[0].name, "Imperio");
    EXPECT_EQ(outM.size(), 1u);
    EXPECT_EQ(outM[0].size(), 2u);
}
