#ifndef PRIEST_H_
#define PRIEST_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Citizen.h"
#include "PriestCommands.h"

class Priest: public Citizen {
private:
    std::unordered_map<std::string, std::unique_ptr<NPCCommand>> commands;

public:
    Priest(uint32_t id, Position pos): Citizen(id, pos, NPCType::PRIEST) {
        // El sacerdote "se guarda" únicamente lo que sabe responder
        commands["heal"] = std::make_unique<HealCommand>();
        commands["resurrect"] = std::make_unique<ResurrectCommand>();
    }

    // void interact(Interactable& source, const std::string& action,
    //               const std::vector<std::string>& params) override {

    //     // 1. Verificamos que la fuente sea un Player
    //     Player* player = dynamic_cast<Player*>(&source);
    //     if (!player)
    //         return;

    //     // 2. Validar que la distancia entre el jugador y el sacerdote sea <= 2 baldosas
    //     if (get_distance_to(player) > 2)
    //         return;

    //     // 3. Enrutamiento dinámico
    //     auto it = commands.find(action);
    //     if (it != commands.end()) {
    //         it->second->execute(*player, params);
    //     }
    // }
};

#endif  // PRIEST_H_
