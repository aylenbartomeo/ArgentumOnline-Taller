#ifndef CLAN_COMMAND_DTO_H
#define CLAN_COMMAND_DTO_H

#include <cstdint>
#include <string>

enum class ClanCommandType : uint8_t {
    FOUND = 0,   // /fundar-clan <nombre>
    JOIN = 1,    // /unirse-clan <nombre>
    LEAVE = 2,   // /dejar-clan
    REVIEW = 3,  // /revisar-clan
    ACCEPT = 4,  // /aceptar-clan <nick>
    REJECT = 5,  // /rechazar-clan <nick>
    BAN = 6,     // /clan-ban <nick>
    KICK = 7,    // /clan-kick <nick>
};

struct ClanCommandDTO {
    ClanCommandType type;
    std::string arg1;  // clanName o targetNick según el comando
    uint32_t targetDbId = 0;
};

#endif
