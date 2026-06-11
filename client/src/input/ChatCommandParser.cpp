#include "ChatCommandParser.h"

#include <utility>

ChatCommandParser::ChatCommandParser(): selectedSlotProvider([]() { return -1; }) {
    registerHandlers();
}

ChatCommandParser::ChatCommandParser(SlotProvider slotProvider):
        selectedSlotProvider(std::move(slotProvider)) {
    registerHandlers();
}

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

    handlers["/consumir"] = [this](const std::string&) -> CommandVariant {
        int slot = selectedSlotProvider();
        if (slot < 0) {
            return ChatDTO{"__INVALID_NO_SLOT__"};
        }
        return UseItemDTO{static_cast<uint8_t>(slot)};
    };

    // === Comando /tirar: usa el slot seleccionado del inventario ===
    handlers["/tirar"] = [this](const std::string& args) -> CommandVariant {
        int slot = selectedSlotProvider();
        if (slot < 0) {
            // Sin slot seleccionado: retornamos un ChatDTO vacío como señal
            // de error (el caller va a ver nullopt vía parse())
            return ChatDTO{"__INVALID_NO_SLOT__"};
        }

        DropItemDTO dto;
        dto.slot = static_cast<uint8_t>(slot);

        if (args.empty()) {
            // Sin argumento: tirar todo el stack (el server interpreta amount=0)
            dto.amount = 0;
        } else {
            try {
                unsigned long parsed = std::stoul(args);
                if (parsed == 0) {
                    // "/tirar 0" explícito: cantidad inválida
                    return ChatDTO{"__INVALID_ZERO_AMOUNT__"};
                }
                dto.amount = static_cast<uint16_t>(parsed);
            } catch (...) {
                return ChatDTO{"__INVALID_AMOUNT_PARSE__"};
            }
        }
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
