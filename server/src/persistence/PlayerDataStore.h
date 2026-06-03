#ifndef PLAYER_DATA_STORE_H
#define PLAYER_DATA_STORE_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../common/utils/position.h"
#include "../../common/utils/types.h"
// Struct de tamaño fijo para persistencia binaria.
// El tamaño se fija a 128 bytes para permitir expansión futura.
#pragma pack(push, 1)
struct PlayerPersistData {
    // Ubicacion e identidad
    uint32_t dbId;
    int32_t posX;
    int32_t posY;
    uint16_t hp;
    uint16_t mana;
    uint16_t level;
    uint32_t exp;
    uint32_t gold;
    uint8_t stateId;
    uint8_t race;
    uint8_t characterClass;
    uint8_t inventorySize;
    struct SlotData {
        uint32_t item_id;
        uint16_t amount;
    } inventory[16];
    uint8_t _pad[2];
};
#pragma pack(pop)
static_assert(sizeof(PlayerPersistData) == 128, "PlayerPersistData must be 128 bytes");

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
