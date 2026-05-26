#ifndef PLAYER_DATA_STORE_H
#define PLAYER_DATA_STORE_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "../../common/utils/position.h"

// Struct de tamaño fijo para persistencia binaria.
// El tamaño se fija a 128 bytes para permitir expansión futura.
struct PlayerPersistData {
    uint32_t dbId;
    int32_t posX;
    int32_t posY;
    // Espacio reservado para expansión futura (HP, mana, level, exp, gold, etc.)
    char reserved[116];
};
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
