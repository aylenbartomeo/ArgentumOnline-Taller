#ifndef MERCHANT_H_
#define MERCHANT_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "server/src/model/entities/citizen.h"

#include "MerchantCommands.h"

class Merchant: public Citizen {
private:
    std::unordered_map<std::string, std::unique_ptr<NPCCommand>> commands;

public:
    Merchant(uint32_t id, Position pos): Citizen(id, pos, NPCType::MERCHANT) {
        // El comerciante "se guarda" únicamente lo que sabe responder
        commands["buy"] = std::make_unique<BuyCommand>();
        commands["sell"] = std::make_unique<SellCommand>();
    }

    void interact(Interactable& source, const std::string& action,
                  const std::vector<std::string>& params) override {

        // 1. Verificamos que la fuente sea un Player
        Player* player = dynamic_cast<Player*>(&source);
        if (!player)
            return;

        // 2. Validar que la distancia entre el jugador y el merchant sea <= 2 baldosas
        if (get_distance_to(player) > 2)
            return;

        // 3. Enrutamiento dinámico
        auto it = commands.find(action);
        if (it != commands.end()) {
            it->second->execute(*player, params);
        }
    }
};

#endif  // MERCHANT_H_
