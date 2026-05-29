#ifndef PRIEST_COMMANDS_H_
#define PRIEST_COMMANDS_H_

#include <string>
#include <vector>

#include "../entities/Player.h"

#include "NPCCommand.h"

class HealCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& /*params*/) override {
        if (player.isDead())
            return false;
        // player.heal();
        return true;  // TODO: Conectar con player.heal()
    }
};

class ResurrectCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& /*params*/) override {
        if (!player.isDead())
            return false;
        // player.resurrect();
        return true;  // TODO: Conectar con player.resurrect()
    }
};

#endif  // PRIEST_COMMANDS_H_
