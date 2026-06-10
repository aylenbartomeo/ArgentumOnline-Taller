#ifndef COMBAT_NOTIFIER_H
#define COMBAT_NOTIFIER_H

#include <cstdint>
#include "server/src/model/events/EventPublisher.h"
#include "server/src/model/entities/Player.h"
#include "server/src/model/entities/Monster.h"

#include "server/src/model/systems/CombatTypes.h"

class CombatNotifier {
private:
    EventPublisher& eventPublisher;
    ICombatEventCallback& callback;

public:
    CombatNotifier(EventPublisher& ep, ICombatEventCallback& cb);

    void notifyCombatResult(const Attackable& attacker, const Attackable& target, const CombatResult& res);
};

#endif // COMBAT_NOTIFIER_H
