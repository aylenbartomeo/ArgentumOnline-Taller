#include "PlayerMock.h"

PlayerMock::PlayerMock(uint32_t id, 
                const std::string& name, 
                const RaceConfig& race, 
                const CharacterClassConfig& characterClass,
                const PlayerConfig& playerBase) :
        id(id), 
        name(name), 
        pos({0, 0}), 
        // Stats ahora solo maneja combate (sin max_gold)
        stats(race, characterClass, playerBase),
        // Inventario ahora absorbe la economía: 20 slots, 5000 seguro, 100000 tope máximo
        inventory(20, 5000, 100000),
        equipment(),
        bank(50, 999999),
        state(),
        combat(stats, state) {}

// Constructor de TEST: Permite pasarle un FormulaEngine controlado para manejar la cuestion
// de valores random
PlayerMock::PlayerMock(uint32_t id, const std::string& name, const RaceConfig& race, 
               const CharacterClassConfig& characterClass,
               const PlayerConfig& playerBase,
               const FormulaEngine& testEngine)
        : id(id), 
        name(name), 
        pos({0, 0}), 
        stats(race, characterClass, playerBase, testEngine),
        inventory(20, 5000, 100000),
        equipment(),
        bank(50, 999999),
        state(),
        combat(stats, state) {}