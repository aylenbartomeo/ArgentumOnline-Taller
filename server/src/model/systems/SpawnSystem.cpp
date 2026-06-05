#include "SpawnSystem.h"
#include <random>

SpawnSystem::SpawnSystem(MonsterConfigs configs, float cooldownMs, size_t maxMon)
    : monsterConfigs(std::move(configs)), respawnCooldownMs(cooldownMs), 
      timeSinceLastSpawnMs(0.0f), maxMonsters(maxMon) {}

std::vector<SpawnRequest> SpawnSystem::getInitialSpawns(const Map& map) const {
    std::vector<SpawnRequest> requests;
    if (monsterConfigs.empty()) return requests;

    for (const auto& spawn : map.getMonsterSpawns()) {
        auto it = monsterConfigs.find(spawn.type);
        if (it != monsterConfigs.end()) {
            requests.push_back({spawn.type, spawn.pos, &it->second});
        }
    }
    return requests;
}

std::vector<SpawnRequest> SpawnSystem::tick(float deltaTime, size_t currentMonsterCount,
                                            const Map& map,
                                            const EntityManager& entityManager) {
    std::vector<SpawnRequest> requests;
    if (monsterConfigs.empty()) return requests;

    timeSinceLastSpawnMs += deltaTime;
    if (timeSinceLastSpawnMs >= respawnCooldownMs) {
        timeSinceLastSpawnMs = 0.0f; // Reiniciamos el contador
        if (currentMonsterCount < maxMonsters) {
            auto posOpt = findValidSpawnPosition(5, map, entityManager);
            if (posOpt) {
                std::vector<NPCType> types;
                types.reserve(monsterConfigs.size());
                for (const auto& pair: monsterConfigs) {
                    types.push_back(pair.first);
                }

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<size_t> dist(0, types.size() - 1);
                NPCType chosen = types[dist(gen)];

                auto it = monsterConfigs.find(chosen);
                if (it != monsterConfigs.end()) {
                    requests.push_back({chosen, *posOpt, &it->second});
                }
            }
        }
    }
    return requests;
}

std::optional<Position> SpawnSystem::findValidSpawnPosition(int maxAttempts, const Map& map, const EntityManager& entityManager) const {
    if (maxAttempts <= 0) return std::nullopt;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distX(0, map.widthLimit() - 1);
    std::uniform_int_distribution<int> distY(0, map.heightLimit() - 1);

    const int playerProximity = 20;  // tiles
    const int playerProximitySq = playerProximity * playerProximity;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        Position candidate{distX(gen), distY(gen)};

        if (map.isSafeZone(candidate.x, candidate.y)) continue;
        if (!map.canMoveTo(candidate)) continue;
        if (map.findNPCNear(candidate, 2).has_value()) continue;

        bool playerNear = false;
        for (const auto& [id, player]: entityManager.getPlayers()) {
            const Position& pp = player->getPosition();
            int dx = pp.x - candidate.x;
            int dy = pp.y - candidate.y;
            int dsq = dx * dx + dy * dy;
            if (dsq <= playerProximitySq) {
                playerNear = true;
                break;
            }
        }
        if (playerNear) continue;

        return candidate;
    }
    return std::nullopt;
}
