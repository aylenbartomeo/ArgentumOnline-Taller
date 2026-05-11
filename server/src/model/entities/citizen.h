#ifndef CITIZEN_H
#define CITIZEN_H

#include <cstdint>
#include <string>
#include <vector>

#include "../utils/position.h"
#include "../utils/types.h"
#include "interfaces/interactable.h"

class Citizen: public Interactable {
private:
    uint32_t id;
    Position pos;
    NPCType type;

public:
    Citizen(uint32_t id, Position pos, NPCType type);

    void interact(Interactable& interactable, const std::string& action,
                  const std::vector<std::string>& params) override;
};

#endif
