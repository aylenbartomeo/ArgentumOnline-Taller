#ifndef BANKER_H_
#define BANKER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../commands/BankerCommands.h"
#include "server/src/model/entities/Citizen.h"

class Banker: public Citizen {
private:
    std::unordered_map<std::string, std::unique_ptr<NPCCommand>> commands;

public:
    Banker(uint32_t id, Position pos): Citizen(id, pos, NPCType::BANKER) {
        // El cajero "se guarda" únicamente lo que sabe responder
        commands["deposit"] = std::make_unique<DepositCommand>();
        commands["withdraw"] = std::make_unique<WithdrawCommand>();
    }

    void interact(Interactable& source, const std::string& action,
                  const std::vector<std::string>& params) override {

        // 1. Verificamos que la fuente sea un Player
        Player* player = dynamic_cast<Player*>(&source);
        if (!player)
            return;

        // 2. Validar distancia
        if (getPosition().chebyshev_distance_to(player->getPosition()) > 2)
            return;

        // 3. Enrutamiento dinámico
        auto it = commands.find(action);
        if (it != commands.end()) {
            it->second->execute(*player, params);
        }
    }
};

#endif  // BANKER_H_
