#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <algorithm>
#include <cstdint>

#include "../../Map.h"
#include "model/clan/ClanRepository.h"
#include "model/entities/EntityManager.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/events/EventPublisher.h"
#include "server/src/model/interfaces/CombatStrategies.h"
#include "server/src/model/systems/ClanBonusCalculator.h"
#include "server/src/model/systems/CombatNotifier.h"
#include "server/src/model/systems/CombatTypes.h"

class IHitEffect;
class Weapon;

class CombatSystem {
private:
    Map& map;
    EntityManager& entityManager;
    ClanRepository& clanRepo;
    EventPublisher& eventPublisher;
    ICombatEventCallback& callback;
    bool enforceFairPlay;

    ClanBonusCalculator clanBonusCalc;
    CombatNotifier notifier;

    CombatResult resolveCombat(const Attackable& attacker, Attackable& target,
                               const AttackParams& params);

    CombatResult processAttack(const Monster& attacker, Attackable& target);

    CombatResult processAttack(Player& attacker, Attackable& target, float attackBonus,
                               float defenseBonus);

public:
    CombatSystem(Map& map, EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                 ICombatEventCallback& cb, bool enforceFairPlay);
    void setFairPlayRules(bool enforce) { enforceFairPlay = enforce; }

    void playerAttack(uint32_t attackerDbId, uint32_t targetDbId);
    void monsterAttack(const Monster& monster, Player& target);

    CombatModifiers buildModifiers(uint32_t attackerDbId, const Attackable* target) const;

    // Aplica el daño final sin validar distancia (para impactos ya confirmados, ej: proyectiles).
    CombatResult applyDamageEffect(const Attackable& attacker, Attackable& target,
                                   const AttackParams& params);

    bool areClanmates(uint32_t dbId1, uint32_t dbId2) const;

    // Aplica la curacion final
    CombatResult applyHealEffect(Player& target);

    // Punto de entrada para el impacto diferido de un proyectil.
    void onProjectileHit(Player& attacker, Attackable& target, IHitEffect* hitEffect,
                         const CombatModifiers& modifiers, const Weapon& weapon);
};

#endif
