#ifndef PLAYER_FACTORY_H
#define PLAYER_FACTORY_H

#include <memory>
#include <string>
#include <cstdint>
#include "PlayerMock.h"
#include "config/CharacterConfig.h"

class PlayerFactory {
private:
    // Almacena las tablas de balance del servidor
    CharacterConfigs serverConfigs;

public:
    // Constructor principal: se alimenta de las configuraciones del servidor
    explicit PlayerFactory(const CharacterConfigs& configs) : serverConfigs(configs) {}

    // ⚔️ MÉTODO DE PRODUCCIÓN: Busca en los mapas reales usando Raza y Clase del juego
    std::unique_ptr<PlayerMock> createPlayer(uint32_t id, const std::string& name, Race race, CharacterClass cls) {
        // Lookups seguros en las configuraciones del servidor
        const PlayerConfig& base = serverConfigs.player;
        const RaceConfig& raceCfg = serverConfigs.races[race];
        const CharacterClassConfig& classCfg = serverConfigs.classes[cls];

        // Retorna el jugador usando el constructor de producción (usa el Singleton de FormulaEngine por defecto)
        return std::make_unique<PlayerMock>(id, name, raceCfg, classCfg, base);
    }

    // 🧪 MÉTODO DE TEST: Permite inyectar un FormulaEngine mockeado/controlado para evitar aleatoriedad
    std::unique_ptr<PlayerMock> createTestPlayer(uint32_t id, const std::string& name, Race race, CharacterClass cls, const FormulaEngine& testEngine) {
        const PlayerConfig& base = serverConfigs.player;
        const RaceConfig& raceCfg = serverConfigs.races[race];
        const CharacterClassConfig& classCfg = serverConfigs.classes[cls];

        // Llama al segundo constructor de tu clase (el que inyecta el motor de test)
        return std::make_unique<PlayerMock>(id, name, raceCfg, classCfg, base, testEngine);
    }
};

#endif // PLAYER_FACTORY_H
