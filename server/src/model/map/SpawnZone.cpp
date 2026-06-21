#include "SpawnZone.h"

#include <random>

SpawnZone::SpawnZone(int x, int y, int width, int height, ZoneType type,
                     const std::vector<NPCType>& allowedTypes, size_t maxMonsters,
                     float cooldownMs):
        x(x),
        y(y),
        width(width),
        height(height),
        zoneType(type),
        allowedMonsterTypes(allowedTypes),
        maxMonsters(maxMonsters),
        currentMonsterCount(0),
        respawnCooldownMs(cooldownMs),
        timeSinceLastSpawnMs(cooldownMs) {
    // Inicializamos timeSinceLastSpawnMs en cooldownMs para que el primer tick pueda spawnear si
    // hay espacio
}

bool SpawnZone::contains(const Position& pos) const {
    if (zoneType == ZoneType::NORMAL) {
        // La zona normal cubre todo el mapa (los límites se chequean en getRandomSpawnPosition),
        // pero la contención específica se delega o asume true si no está en otras zonas.
        // Para este propósito, decimos que está en la zona si cae aquí.
        return true;
    }
    return pos.x >= x && pos.x <= x + width && pos.y >= y && pos.y <= y + height;
}

bool SpawnZone::tickCooldown(float deltaMs) {
    if (!needsSpawn())
        return false;

    timeSinceLastSpawnMs += deltaMs;
    if (timeSinceLastSpawnMs >= respawnCooldownMs) {
        timeSinceLastSpawnMs = 0.0f;
        return true;
    }
    return false;
}

bool SpawnZone::needsSpawn() const {
    return currentMonsterCount < maxMonsters && !allowedMonsterTypes.empty();
}

void SpawnZone::incrementCount() {
    if (currentMonsterCount < maxMonsters) {
        currentMonsterCount++;
    }
}

void SpawnZone::decrementCount() {
    if (currentMonsterCount > 0) {
        currentMonsterCount--;
    }
}

std::optional<NPCType> SpawnZone::getRandomAllowedType() const {
    if (allowedMonsterTypes.empty())
        return std::nullopt;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, allowedMonsterTypes.size() - 1);

    return allowedMonsterTypes[dist(gen)];
}

std::optional<Position> SpawnZone::getRandomSpawnPosition(int maxAttempts, const Map& map) const {
    if (maxAttempts <= 0)
        return std::nullopt;

    std::random_device rd;
    std::mt19937 gen(rd());

    int minX = (zoneType == ZoneType::NORMAL) ? 0 : x;
    int maxX = (zoneType == ZoneType::NORMAL) ? map.widthLimit() - 1 : x + width;
    int minY = (zoneType == ZoneType::NORMAL) ? 0 : y;
    int maxY = (zoneType == ZoneType::NORMAL) ? map.heightLimit() - 1 : y + height;

    // Clamping just in case
    if (maxX >= map.widthLimit())
        maxX = map.widthLimit() - 1;
    if (maxY >= map.heightLimit())
        maxY = map.heightLimit() - 1;

    std::uniform_int_distribution<int> distX(minX, maxX);
    std::uniform_int_distribution<int> distY(minY, maxY);

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        Position candidate{distX(gen), distY(gen)};

        if (map.isSafeZone(candidate.x, candidate.y))
            continue;
        if (!map.canMoveTo(candidate))
            continue;
        if (map.findNPCNear(candidate, 2).has_value())
            continue;

        bool inBossZone = std::any_of(
                map.getBossZones().begin(), map.getBossZones().end(), [&](const auto& bz) {
                    return candidate.x >= bz.x && candidate.x <= bz.x + bz.width &&
                           candidate.y >= bz.y && candidate.y <= bz.y + bz.height;
                });
        if (inBossZone)
            continue;

        if (zoneType == ZoneType::NORMAL) {
            // Check if it's in any specific zone (forest, desert)
            bool inSpecificZone = std::any_of(
                    map.getZoneRects().begin(), map.getZoneRects().end(), [&](const auto& zRect) {
                        return zRect.type != ZoneType::NORMAL && candidate.x >= zRect.x &&
                               candidate.x <= zRect.x + zRect.width && candidate.y >= zRect.y &&
                               candidate.y <= zRect.y + zRect.height;
                    });
            if (inSpecificZone)
                continue;
        }

        // Podríamos revisar la proximidad a jugadores, pero como se pide en el viejo SpawnSystem
        // Aquí no tenemos acceso directo al EntityManager tan fácil sin acoplar SpawnZone.
        // Lo dejamos delegado a que si está libre, puede spawnear (similar a BossZone).

        return candidate;
    }
    return std::nullopt;
}
