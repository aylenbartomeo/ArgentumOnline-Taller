#ifndef I_WORLD_CONTEXT_H
#define I_WORLD_CONTEXT_H

#include <cstdint>
#include <string>

// Permite al controlador consultar datos del mundo sin conocer la clase World.
class IWorldContext {
public:
    virtual ~IWorldContext() = default;

    // Devuelve el nivel del jugador (0 si no está online)
    virtual uint16_t getPlayerLevel(uint32_t dbId) const = 0;

    // Traduce un nombre de jugador a su dbId (0 si no está online)
    virtual uint32_t resolveNickToDbId(const std::string& nick) const = 0;

    virtual std::optional<std::string> getPlayerUsername(uint32_t dbId) const = 0;
};

#endif
