#ifndef ZONE_SPAWN_SYSTEM_H
#define ZONE_SPAWN_SYSTEM_H

#include <vector>
#include <unordered_map>
#include <memory>

#include "../../Map.h"
#include "../../config/MonsterConfig.h"
#include "../entities/EntityManager.h"
#include "../map/SpawnZone.h"
#include "SpawnRequest.h"

class ZoneSpawnSystem {
private:
    MonsterConfigs monsterConfigs;
    std::vector<SpawnZone> zones;
    size_t totalMaxMonsters;
    float respawnCooldownMs;
    
    // Mapeo inverso de ZoneType a los NPCTypes permitidos según la configuración
    std::unordered_map<ZoneType, std::vector<NPCType>> zoneAllowedTypes;

    void distributeMonsters();

public:
    explicit ZoneSpawnSystem(MonsterConfigs configs = {}, float cooldownMs = 15000.0f, size_t maxMon = 50);

    // Inicializa las zonas a partir de los datos del mapa
    void initializeZones(const Map& map);

    // Genera las posiciones y tipos de monstruos iniciales al cargar el mundo
    std::vector<SpawnRequest> getInitialSpawns(const Map& map);

    // Tick del sistema, devuelve los monstruos que deben spawnear en este frame
    std::vector<SpawnRequest> tick(float deltaTime, const Map& map);

    // Notificación de que un monstruo murió en cierta posición, para decrementar la zona correcta
    void onMonsterDeath(const Position& pos);

    const MonsterConfigs& getConfigs() const { return monsterConfigs; }
};

#endif // ZONE_SPAWN_SYSTEM_H
