#include "server/src/model/systems/CombatNotifier.h"
#include <string>

CombatNotifier::CombatNotifier(EventPublisher& ep, ICombatEventCallback& cb)
    : eventPublisher(ep), callback(cb) {}

void CombatNotifier::notifyCombatResult(const Attackable& attacker, const Attackable& target, const CombatResult& res) {
    if (!res.attackHappened || res.isPending)
        return;

    const Player* pAttacker = dynamic_cast<const Player*>(&attacker);
    const Monster* mAttacker = dynamic_cast<const Monster*>(&attacker);

    const Player* pTarget = dynamic_cast<const Player*>(&target);
    const Monster* mTarget = dynamic_cast<const Monster*>(&target);

    if (res.evaded) {
        // Notificar atacante (si es player)
        if (pAttacker) {
            eventPublisher.sendTo(pAttacker->getDbId(), "¡" + target.getName() + " evadió tu ataque!");
        }
        // Notificar target (si es player)
        if (pTarget) {
            eventPublisher.sendTo(pTarget->getDbId(), "¡Evadiste el ataque de " + attacker.getName() + "!");
        }
    } else {
        // Notificar atacante (si es player)
        if (pAttacker) {
            std::string critMsg = res.critical ? " ¡GOLPE CRITICO!" : "";
            eventPublisher.sendTo(pAttacker->getDbId(), "¡Le hiciste " + std::to_string(res.damage) +
                                                        " de dano a " + target.getName() + "!" + critMsg);
        }

        // Notificar target (si es player) y chequear si muere
        if (pTarget) {
            eventPublisher.sendTo(pTarget->getDbId(), "¡Recibiste " + std::to_string(res.damage) +
                                                      " de dano de " + attacker.getName() + "!");

            if (pTarget->isDead()) {
                std::string deathMsg = attacker.getName() + " ha asesinado a " + pTarget->getName() + "!";
                eventPublisher.broadcast(deathMsg);
                callback.onPlayerDeath(pTarget->getDbId());
            }
        }

        // Chequear muerte del monster target
        if (pAttacker && mTarget && mTarget->isDead()) {
            callback.onMonsterDeath(*mTarget, pAttacker->getDbId());
        }
    }
}
