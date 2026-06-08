#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <cstdint>

#include "../../Map.h"
#include "model/clan/ClanRepository.h"
#include "model/entities/EntityManager.h"
#include "model/entities/Monster.h"
#include "model/entities/Player.h"
#include "model/events/EventPublisher.h"

class ICombatEventCallback {
public:
    virtual void onMonsterDeath(const Monster& monster, uint32_t killerDbId) = 0;
    virtual void onPlayerDeath(uint32_t dbId) = 0;
    virtual ~ICombatEventCallback() = default;
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

public:
    CombatSystem(Map& map, EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                 ICombatEventCallback& cb, bool enforceFairPlay);
    void setFairPlayRules(bool enforce) { enforceFairPlay = enforce; }

    void playerAttack(uint32_t attackerDbId, uint32_t targetDbId);
    void monsterAttack(const Monster& monster, Player& target);

    bool areClanmates(uint32_t dbId1, uint32_t dbId2) const;
    int countNearbyClanmates(uint32_t dbId, int range) const;
};

#endif  // COMBAT_SYSTEM_H
