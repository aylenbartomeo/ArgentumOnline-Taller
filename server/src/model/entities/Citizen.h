#ifndef CITIZEN_H
#define CITIZEN_H

#include <cstdint>
#include <string>
#include <vector>

#include "../interfaces/interactable.h"
#include "../utils/position.h"
#include "../utils/types.h"

// class Player;

class Citizen: public Interactable {
private:
    uint32_t id;
    Position pos;
    NPCType type;

public:
    Citizen(uint32_t id, Position pos, NPCType type);

    uint32_t getId() const { return id; }
    Position getPosition() const { return pos; }
    NPCType getType() const { return type; }

    void interact(Interactable& interactable, const std::string& action,
                  const std::vector<std::string>& params) override;

    // int get_distance_to(const Player* player) const;
};

#endif
