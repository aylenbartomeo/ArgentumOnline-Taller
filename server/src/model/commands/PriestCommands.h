#ifndef PRIEST_COMMANDS_H_
#define PRIEST_COMMANDS_H_

#include <string>
#include <vector>

#include "model/entities/Player.h"

#include "NPCCommand.h"

class HealCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& params) override {
        if (player.is_dead())
            return false;
        player.heal();
        return true;
    }
};

class ResurrectCommand: public NPCCommand {
public:
    bool execute(Player& player, const std::vector<std::string>& params) override {
        if (!player.is_dead())
            return false;
        player.resurrect();
        return true;
    }
};

#endif  // PRIEST_COMMANDS_H_
