#include "ChatCommandParser.h"

ChatCommandParser::ChatCommandParser() { registerHandlers(); }

void ChatCommandParser::registerHandlers() {
    // === Comandos sin argumentos ===
    handlers["/tomar"] = [](const std::string&) -> CommandVariant { return GrabItemDTO{}; };

    handlers["/meditar"] = [](const std::string&) -> CommandVariant { return MeditateDTO{}; };

    handlers["/listar"] = [](const std::string&) -> CommandVariant {
        return NpcCommandDTO{LIST, ""};
    };

    handlers["/resucitar"] = [](const std::string&) -> CommandVariant { return ResurrectDTO{}; };

    // === Comandos NPC (necesitan 1 argumento opcional/obligatorio) ===
    handlers["/curar"] = [](const std::string&) -> CommandVariant {
        return NpcCommandDTO{HEAL, ""};
    };
    handlers["/comprar"] = [](const std::string& args) -> CommandVariant {
        return NpcCommandDTO{BUY, args};
    };
    handlers["/vender"] = [](const std::string& args) -> CommandVariant {
        return NpcCommandDTO{SELL, args};
    };
    handlers["/depositar"] = [](const std::string& args) -> CommandVariant {
        return NpcCommandDTO{DEPOSIT, args};
    };
    handlers["/retirar"] = [](const std::string& args) -> CommandVariant {
        return NpcCommandDTO{WITHDRAW, args};
    };

    // === Comandos de Clan ===
    handlers["/fundar-clan"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::FOUND, args};
    };
    handlers["/unirse"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::JOIN, args};
    };
    handlers["/revisar-clan"] = [](const std::string&) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::REVIEW, ""};
    };
    handlers["/clan-aceptar"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::ACCEPT, args};
    };
    handlers["/clan-rechazar"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::REJECT, args};
    };
    handlers["/clan-ban"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::BAN, args};
    };
    handlers["/dejar-clan"] = [](const std::string&) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::LEAVE, ""};
    };
    handlers["/clan-kick"] = [](const std::string& args) -> CommandVariant {
        return ClanCommandDTO{ClanCommandType::KICK, args};
    };

    // === Comandos pendientes / TODO ===
    handlers["/tirar"] = [](const std::string& args) -> CommandVariant {
        // TODO: Mapear al slot seleccionado cuando el cliente maneje inventario.
        // Por ahora lo mandamos con slot 0 temporalmente, u omitimos su uso real.
        uint16_t amount = 1;
        if (!args.empty()) {
            try {
                amount = static_cast<uint16_t>(std::stoul(args));
            } catch (...) {}
        }
        DropItemDTO dto;
        dto.slot = 0;  // Stub
        dto.amount = amount;
        return dto;
    };
}

std::optional<CommandVariant> ChatCommandParser::parse(const std::string& text) const {
    if (text.empty()) {
        return std::nullopt;
    }

    // 1. Mensaje privado: @nick mensaje
    if (text[0] == '@') {
        const auto space = text.find(' ', 1);
        if (space != std::string::npos && space > 1) {
            PrivateChatDTO dto;
            dto.recipientNick = text.substr(1, space - 1);
            dto.message = text.substr(space + 1);
            if (!dto.message.empty()) {
                return dto;
            }
        }
        return std::nullopt;
    }

    // 2. ¿Empieza con '/'? -> buscar comando
    if (text[0] == '/') {
        auto spacePos = text.find(' ');
        std::string cmd = text.substr(0, spacePos);
        std::string args = (spacePos != std::string::npos) ? text.substr(spacePos + 1) : "";

        auto it = handlers.find(cmd);
        if (it != handlers.end()) {
            return it->second(args);
        }
        return std::nullopt;
    }

    // 3. Texto libre -> chat general
    return ChatDTO{text};
}
