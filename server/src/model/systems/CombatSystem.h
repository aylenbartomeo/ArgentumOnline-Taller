#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

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
};

// Struct intermediario que desacopla la estrategia de combate
// de la fuente de los parámetros (Weapon para un Player, config para un Monster).
struct AttackParams {
    uint16_t minDamage;
    uint16_t maxDamage;
    int attackRange;
    int manaCost;               // 0 para ataques físicos y monstruos
    bool isMagic;               // true para ataques mágicos (determina la estrategia)
    float attackBonus = 1.0f;   // multiplicador de ataque por bonus grupal de clan
    float defenseBonus = 1.0f;  // multiplicador de defensa por bonus grupal de clan
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

    // Lógica compartida de combate. Retorna el daño final infligido, o -1 si
    // el ataque no se concretó (fuera de rango, esquivado, target muerto).
    CombatResult resolveCombat(const Attackable& attacker, Attackable& target,
                               const AttackParams& params);

    // Monster ataca a cualquier entidad (típicamente un Player)
    CombatResult processAttack(const Monster& attacker, Attackable& target);

    // Variante con bonuses de clan pre-calculados por World
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

    // Este método se encarga estrictamente de calcular el daño final y aplicarlo.
    // Es el que invocará polimórficamente tu futuro IHitEffect o el ProjectileSystem.
    CombatResult applyDamageEffect(Attackable& attacker, Attackable& target, 
                                   const AttackParams& params);
    // Podria haber un
    // CombatResult applyHealEffect(Attackable& attacker, Attackable& target);

    void onProjectileHit(Attackable& attacker, Attackable& target, 
                     IHitEffect* hitEffect, const CombatModifiers& modifiers, 
                     const Weapon& weapon);
};

#endif
