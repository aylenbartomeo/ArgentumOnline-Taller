#include "ChatCommandParser.h"

#include <utility>

// ─── Constructores ────────────────────────────────────────────────────────────
ChatCommandParser::ChatCommandParser():
        selectedSlotProvider([]() { return -1; }), targetProvider([]() { return std::nullopt; }) {
    registerHandlers();
}

ChatCommandParser::ChatCommandParser(SlotProvider slotProvider, TargetProvider targetProvider):
        selectedSlotProvider(std::move(slotProvider)), targetProvider(std::move(targetProvider)) {
    registerHandlers();
}

// ─── Helpers de registro ──────────────────────────────────────────────────────
void ChatCommandParser::registerNoArgCommand(const std::string& cmd, CommandVariant dto) {
    handlers[cmd] = [dto = std::move(dto)](const std::string&) -> CommandVariant { return dto; };
}

void ChatCommandParser::registerNpcCommand(const std::string& cmd, NpcCommandType type) {
    handlers[cmd] = [type](const std::string& args) -> CommandVariant {
        return NpcCommandDTO{type, args, 0};
    };
}

void ChatCommandParser::registerClanCommand(const std::string& cmd, ClanCommandType type,
                                            bool needsArgs) {
    if (needsArgs) {
        handlers[cmd] = [type](const std::string& args) -> CommandVariant {
            return ClanCommandDTO{type, args};
        };
    } else {
        handlers[cmd] = [type](const std::string&) -> CommandVariant {
            return ClanCommandDTO{type, ""};
        };
    }
}

// ─── Registro de comandos ─────────────────────────────────────────────────────
void ChatCommandParser::registerHandlers() {
    // Comandos sin argumentos
    registerNoArgCommand("/tomar", GrabItemDTO{});
    registerNoArgCommand("/meditar", MeditateDTO{});
    registerNoArgCommand("/listar", NpcCommandDTO{LIST, ""});
    registerNoArgCommand("/resucitar", ResurrectDTO{});

    // Comandos NPC
    registerNpcCommand("/curar", HEAL);
    registerNpcCommand("/comprar", BUY);
    registerNpcCommand("/vender", SELL);
    registerNpcCommand("/depositar", DEPOSIT);
    registerNpcCommand("/retirar", WITHDRAW);

    // Comandos de Clan
    registerClanCommand("/fundar-clan", ClanCommandType::FOUND);
    registerClanCommand("/unirse", ClanCommandType::JOIN);
    registerClanCommand("/revisar-clan", ClanCommandType::REVIEW, false);
    registerClanCommand("/dejar-clan", ClanCommandType::LEAVE, false);
    registerClanCommand("/clan-aceptar", ClanCommandType::ACCEPT);
    registerClanCommand("/clan-rechazar", ClanCommandType::REJECT);
    registerClanCommand("/clan-ban", ClanCommandType::BAN);
    registerClanCommand("/clan-kick", ClanCommandType::KICK);

    // Comando /consumir — requiere slot seleccionado
    handlers["/consumir"] = [this](const std::string&) -> CommandVariant {
        int slot = selectedSlotProvider();
        if (slot < 0) {
            return ChatDTO{"__INVALID_NO_SLOT__"};
        }
        return UseItemDTO{static_cast<uint8_t>(slot)};
    };

    // Comando /tirar — requiere slot seleccionado y cantidad opcional
    handlers["/tirar"] = [this](const std::string& args) -> CommandVariant {
        int slot = selectedSlotProvider();
        if (slot < 0)
            return ChatDTO{"__INVALID_NO_SLOT__"};

        DropItemDTO dto;
        dto.slot = static_cast<uint8_t>(slot);

        if (args.empty()) {
            dto.amount = 0;
            return dto;
        }

        try {
            unsigned long parsed = std::stoul(args);
            if (parsed == 0)
                return ChatDTO{"__INVALID_ZERO_AMOUNT__"};
            dto.amount = static_cast<uint16_t>(parsed);
        } catch (...) {
            return ChatDTO{"__INVALID_AMOUNT_PARSE__"};
        }
        return dto;
    };
}

// ─── Parseo ───────────────────────────────────────────────────────────────────
std::optional<CommandVariant> ChatCommandParser::parsePrivateMessage(const std::string& text) {
    const auto space = text.find(' ', 1);
    if (space == std::string::npos || space <= 1)
        return std::nullopt;

    PrivateChatDTO dto;
    dto.recipientNick = text.substr(1, space - 1);
    dto.message = text.substr(space + 1);

    if (dto.message.empty())
        return std::nullopt;
    return dto;
}

std::optional<CommandVariant> ChatCommandParser::parseSlashCommand(const std::string& text) const {
    auto spacePos = text.find(' ');
    std::string cmd = text.substr(0, spacePos);
    std::string args = (spacePos != std::string::npos) ? text.substr(spacePos + 1) : "";

    auto it = handlers.find(cmd);
    if (it == handlers.end())
        return std::nullopt;
    return it->second(args);
}

std::optional<CommandVariant> ChatCommandParser::parse(const std::string& text) const {
    if (text.empty())
        return std::nullopt;

    if (text[0] == '@')
        return parsePrivateMessage(text);

    if (text[0] == '/')
        return parseSlashCommand(text);

    return ChatDTO{text};
}
