#ifndef NPC_POLICY_H
#define NPC_POLICY_H

#include <optional>
#include <string>

#include "common/include/dto/ClientCommands.h"

// ─── NpcPolicy.h ──────────────────────────────────────────────────────────────
// Tabla de reglas de negocio sobre qué comandos acepta cada tipo de NPC,
// y los mensajes de feedback asociados.

namespace NpcPolicy {

struct Rule {
    std::string feedbackMsg;
};

// Devuelve la regla (con feedback) si el comando es permitido para ese tipo de NPC,
// o std::nullopt si no está permitido.
inline std::optional<Rule> validate(const std::string& npcType, NpcCommandType cmdType) {
    if (npcType == "priest") {
        switch (cmdType) {
            case HEAL:
                return Rule{"[INFO] Pidiendo curación a los dioses..."};
            case BUY:
                return Rule{"[INFO] Comprando artículos sagrados..."};
            case RESPAWN:
                return Rule{"[INFO] Solicitando resurrección..."};
            case LIST:
                return Rule{"[INFO] Te muestro las ofrendas disponibles..."};
            default:
                return std::nullopt;
        }
    }
    if (npcType == "merchant") {
        switch (cmdType) {
            case BUY:
                return Rule{"[INFO] Comprando mercancía..."};
            case SELL:
                return Rule{"[INFO] Vendiendo mercancía..."};
            case LIST:
                return Rule{"[INFO] Te muestro mi mercadería..."};
            default:
                return std::nullopt;
        }
    }
    if (npcType == "banker") {
        switch (cmdType) {
            case DEPOSIT:
                return Rule{"[INFO] Depositando en tu bóveda..."};
            case WITHDRAW:
                return Rule{"[INFO] Retirando de tu bóveda..."};
            case LIST:
                return Rule{"[INFO] Revisando tu bóveda..."};
            default:
                return std::nullopt;
        }
    }
    return std::nullopt;
}

}  // namespace NpcPolicy

#endif  // NPC_POLICY_H
