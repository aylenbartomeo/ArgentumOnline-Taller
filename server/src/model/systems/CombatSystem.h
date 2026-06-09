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

struct CombatModifiers;
class IHitEffect;
class Weapon;

class ICombatEventCallback {
public:
    virtual void onMonsterDeath(const Monster& monster, uint32_t killerDbId) = 0;
    virtual void onPlayerDeath(uint32_t dbId) = 0;
    virtual ~ICombatEventCallback() = default;
};

struct CombatResult {
    bool attackHappened = false;
    bool evaded = false;
    bool critical = false;
    int damage = 0;
    bool isPending = false;
};

struct AttackParams {
    uint16_t minDamage;
    uint16_t maxDamage;
    int attackRange;
    int manaCost;
    bool isMagic;
    float attackBonus = 1.0f;
    float defenseBonus = 1.0f;
};

class CombatSystem {
private:
    Map& map;
    EntityManager& entityManager;
    ClanRepository& clanRepo;
    EventPublisher& eventPublisher;
    ICombatEventCallback& callback;
    bool enforceFairPlay;

    int CLAN_BONUS_RANGE = 5;
    float CLAN_ATTACK_BONUS_PER_MEMBER = 0.05f;
    float CLAN_DEFENSE_BONUS_PER_MEMBER = 0.05f;

    CombatResult resolveCombat(const Attackable& attacker, Attackable& target,
                               const AttackParams& params);

    void notifyCombatResult(const Attackable& attacker, const Attackable& target,
                            const CombatResult& res);

    CombatResult processAttack(const Monster& attacker, Attackable& target);

    CombatResult processAttack(Player& attacker, Attackable& target, float attackBonus,
                               float defenseBonus);

public:
    CombatSystem(Map& map, EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                 ICombatEventCallback& cb, bool enforceFairPlay);
    void setFairPlayRules(bool enforce) { enforceFairPlay = enforce; }

    void playerAttack(uint32_t attackerDbId, uint32_t targetDbId);
    void monsterAttack(const Monster& monster, Player& target);

    bool areClanmates(uint32_t dbId1, uint32_t dbId2) const;
    int countNearbyClanmates(uint32_t dbId, int range) const;

    // Calcula los modificadores de clan para un par atacante/target. (Usado por ProjectileSystem al
    // momento del impacto diferido)
    CombatModifiers buildModifiers(uint32_t attackerDbId, const Attackable* target) const;

    // Aplica el daño final sin validar distancia (para impactos ya confirmados, ej: proyectiles).
    CombatResult applyDamageEffect(const Attackable& attacker, Attackable& target,
                                   const AttackParams& params);

    // Punto de entrada para el impacto diferido de un proyectil.
    void onProjectileHit(Attackable& attacker, Attackable& target, IHitEffect* hitEffect,
                         const CombatModifiers& modifiers, const Weapon& weapon);
};

#endif
