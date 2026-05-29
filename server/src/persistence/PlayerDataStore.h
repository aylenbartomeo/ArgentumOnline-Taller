#ifndef PLAYER_DATA_STORE_H
#define PLAYER_DATA_STORE_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../common/utils/position.h"
#include "../../common/utils/types.h"

static constexpr size_t MAX_INVENTORY_SLOTS = 20;

// Slot de inventario persistente (ID de item + cantidad)
struct PersistedSlot {
    uint32_t itemId{0};
    uint32_t amount{0};
};


// Struct de tamaño fijo para persistencia binaria. (512 bytes)
struct PlayerPersistData {
    // Ubicacion e identidad
    uint32_t dbId;
    int32_t posX;
    int32_t posY;

    // Estadisticas
    uint32_t level{1};
    uint32_t exp{0};
    uint32_t hp{0};
    uint32_t maxHp{0};
    uint32_t mana{0};
    uint32_t maxMana{0};
    uint32_t gold{0};

    // Identidad y Estado
    uint32_t race{0};
    uint32_t charClass{0};
    bool isGhost{false};
    char padding[3]{0, 0, 0};

    // Inventario
    PersistedSlot inventory[MAX_INVENTORY_SLOTS]{};

    // Espacio reservado para expansión futura (Banco, stats de combate extra, clan Id, etc.)
    char reserved[300]{0};
};

static_assert(sizeof(PlayerPersistData) == 512, "PlayerPersistData must be exactly 512 bytes");

class PlayerDataStore {
private:
    // Entrada del índice en disco
    struct PlayerIndexEntry {
        char username[64];
        uint64_t offset;
    };

    static constexpr size_t MAX_USERNAME_LENGTH = 64;

    std::string dataFilePath;
    std::string indexFilePath;

    // Índice cargado en memoria: username -> offset en el archivo de datos
    std::unordered_map<std::string, uint64_t> index;

    // Lee el índice completo del disco a memoria
    void loadIndex();

    // Escribe el índice completo de memoria a disco
    void flushIndex();

public:
    explicit PlayerDataStore(const std::string& persistenceDir = "game_data/");

    // Carga la data de un jugador desde el archivo binario (seek + read)
    std::optional<PlayerPersistData> loadPlayerData(const std::string& username) const;

    // Guarda la data de un jugador. Si no existe, lo crea (append + actualiza índice).
    void savePlayerData(const std::string& username, const PlayerPersistData& data);
};

#endif
