#ifndef WORLD_DATA_STORE_H
#define WORLD_DATA_STORE_H

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "WorldPersistData.h"

class WorldDataStore {
public:
    explicit WorldDataStore(const std::string& worldsBaseDir = "worlds/");

    // Gestión de mundos
    uint32_t createWorld(const std::string& name, const std::string& baseMapPath);
    std::vector<WorldMetadata> listSavedWorlds() const;
    std::optional<WorldMetadata> loadWorldMetadata(uint32_t worldId) const;

    // Persistencia del estado de los monstruos
    void saveMonsters(uint32_t worldId, const std::vector<MonsterPersistData>& monsters);
    std::vector<MonsterPersistData> loadMonsters(uint32_t worldId) const;

    // Persistencia del estado dinámico de los NPCs
    void saveNpcStates(uint32_t worldId,
                   const std::vector<NpcHeaderPersistData>& headers,
                   const std::vector<std::vector<NpcStockPersistData>>& allStocks);

    std::pair<std::vector<NpcHeaderPersistData>, std::vector<std::vector<NpcStockPersistData>>>
        loadNpcStates(uint32_t worldId) const;

    void saveGroundItems(uint32_t worldId, const std::vector<GroundItemPersistData>& items);
    std::vector<GroundItemPersistData> loadGroundItems(uint32_t worldId) const;

    // Persistencia de Clanes
    void saveClans(uint32_t worldId, const std::vector<ClanHeaderPersistData>& headers,
                   const std::vector<std::vector<ClanPlayerPersistData>>& allMembers,
                   const std::vector<std::vector<ClanPlayerPersistData>>& allPending,
                   const std::vector<std::vector<ClanPlayerPersistData>>& allBanned);
    std::tuple<std::vector<ClanHeaderPersistData>, std::vector<std::vector<ClanPlayerPersistData>>,
               std::vector<std::vector<ClanPlayerPersistData>>,
               std::vector<std::vector<ClanPlayerPersistData>>>
            loadClans(uint32_t worldId) const;

    // Persistencia del banco global
    void saveBankAccounts(uint32_t worldId,
                          const std::vector<BankAccountHeaderPersistData>& headers,
                          const std::vector<std::vector<BankSlotPersistData>>& slots);
    std::pair<std::vector<BankAccountHeaderPersistData>,
              std::vector<std::vector<BankSlotPersistData>>>
            loadBankAccounts(uint32_t worldId) const;

    // Ruta de persistencia para PlayerDataStore de un mundo específico
    std::string getWorldDir(uint32_t worldId) const;

private:
    std::string baseDir;
    uint32_t loadNextId() const;
    void saveNextId(uint32_t nextId);
};

#endif  // WORLD_DATA_STORE_H
