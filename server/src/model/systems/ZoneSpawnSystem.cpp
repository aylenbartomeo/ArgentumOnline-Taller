#include "ZoneSpawnSystem.h"

#include <algorithm>
#include <utility>

ZoneSpawnSystem::ZoneSpawnSystem(MonsterConfigs configs, float cooldownMs, size_t maxMon):
        monsterConfigs(std::move(configs)),
        totalMaxMonsters(maxMon),
        respawnCooldownMs(cooldownMs) {

    // Clasificar monstruos por zona según config
    for (const auto& [type, config]: monsterConfigs) {
        if (config.isBoss)
            continue;  // Bosses no van en zonas normales
        ZoneType zt = zoneTypeFromString(config.zone);
        zoneAllowedTypes[zt].push_back(type);
    }
}

void ZoneSpawnSystem::initializeZones(const Map& map) {
    zones.clear();

    const auto& mapZones = map.getZoneRects();

    // Agregar las zonas del mapa (forest, desert)
    for (const auto& rect: mapZones) {
        if (rect.type == ZoneType::NORMAL)
            continue;  // La zona normal se crea al final

        std::vector<NPCType> allowed;
        auto it = zoneAllowedTypes.find(rect.type);
        if (it != zoneAllowedTypes.end()) {
            allowed = it->second;
        }
        zones.emplace_back(rect.x, rect.y, rect.width, rect.height, rect.type, allowed, 0,
                           respawnCooldownMs);
    }

    // Siempre agregamos la zona NORMAL explícita
    std::vector<NPCType> normalAllowed;
    auto normalIt = zoneAllowedTypes.find(ZoneType::NORMAL);
    if (normalIt != zoneAllowedTypes.end()) {
        normalAllowed = normalIt->second;
    }
    zones.emplace_back(0, 0, map.widthLimit(), map.heightLimit(), ZoneType::NORMAL, normalAllowed,
                       0, respawnCooldownMs);

    distributeMonsters();
}

void ZoneSpawnSystem::distributeMonsters() {
    if (zones.empty())
        return;

    size_t count = zones.size();
    size_t basePerZone = totalMaxMonsters / count;
    size_t remainder = totalMaxMonsters % count;

    for (auto& zone: zones) {
        size_t limit = basePerZone;
        if (zone.getType() == ZoneType::NORMAL) {
            limit += remainder;  // El remanente va a la zona normal
        }
        zone.setMaxMonsters(limit);
    }
}

std::vector<SpawnRequest> ZoneSpawnSystem::getInitialSpawns(const Map& map) {
    std::vector<SpawnRequest> requests;
    if (zones.empty())
        return requests;

    // En vez de usar map.getMonsterSpawns(), llenamos las zonas dinámicamente
    // según el maxMonsters de cada una.
    // Hack: Como la firma es const, no podemos incrementar el counter aquí,
    // pero el World lo hace a través de update/addMonster.
    // Ojo: ZoneSpawnSystem actualiza sus contadores en runtime,
    // entonces acá NO lo hacemos. World debe llamar algo...
    // Mejor: no proveemos initialSpawns aquí o lo hacemos y que el World simplemente los añada
    // Y luego el count lo asume el system. Para mantenerlo simple, delegamos al tick
    // el spawn inicial, forzando un cooldown inicial en 0 o llenándolo ahora.
    // Ya que estamos, lo llenamos:

    for (auto& zone: zones) {
        for (size_t i = 0; i < zone.getMaxMonsters(); ++i) {
            auto typeOpt = zone.getRandomAllowedType();
            if (!typeOpt)
                break;

            auto posOpt = zone.getRandomSpawnPosition(10, map);
            if (posOpt) {
                auto it = monsterConfigs.find(*typeOpt);
                if (it != monsterConfigs.end()) {
                    requests.push_back({*typeOpt, *posOpt, &it->second});
                    zone.incrementCount();
                }
            }
        }
    }
    return requests;
}

std::vector<SpawnRequest> ZoneSpawnSystem::tick(float deltaTime, const Map& map) {
    std::vector<SpawnRequest> requests;
    if (zones.empty())
        return requests;

    for (auto& zone: zones) {
        if (zone.tickCooldown(deltaTime)) {
            auto typeOpt = zone.getRandomAllowedType();
            if (!typeOpt)
                continue;

            auto posOpt = zone.getRandomSpawnPosition(5, map);
            if (posOpt) {
                auto it = monsterConfigs.find(*typeOpt);
                if (it != monsterConfigs.end()) {
                    requests.push_back({*typeOpt, *posOpt, &it->second});
                    zone.incrementCount();
                }
            }
        }
    }
    return requests;
}

void ZoneSpawnSystem::onMonsterDeath(const Position& pos) {
    // Buscar la zona específica (no normal) que contiene esta posición
    auto it = std::find_if(zones.begin(), zones.end(), [&](const SpawnZone& zone) {
        return zone.getType() != ZoneType::NORMAL && zone.contains(pos);
    });
    if (it != zones.end()) {
        it->decrementCount();
        return;
    }
    // Si no estaba en ninguna específica, debe haber estado en la normal
    auto normalIt = std::find_if(zones.begin(), zones.end(), [](const SpawnZone& zone) {
        return zone.getType() == ZoneType::NORMAL;
    });
    if (normalIt != zones.end()) {
        normalIt->decrementCount();
        return;
    }
}
