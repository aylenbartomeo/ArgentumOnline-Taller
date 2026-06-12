#ifndef CLAN_BONUS_CALCULATOR_H
#define CLAN_BONUS_CALCULATOR_H

#include <cstdint>

#include "server/src/model/clan/ClanRepository.h"
#include "server/src/model/entities/EntityManager.h"
#include "server/src/model/events/EventPublisher.h"
#include "server/src/model/interfaces/CombatStrategies.h"  // For CombatModifiers

class ClanBonusCalculator {
private:
    EntityManager& entityManager;
    ClanRepository& clanRepo;
    EventPublisher& eventPublisher;

    int CLAN_BONUS_RANGE = 5;
    float CLAN_ATTACK_BONUS_PER_MEMBER = 0.05f;
    float CLAN_DEFENSE_BONUS_PER_MEMBER = 0.05f;

public:
    ClanBonusCalculator(EntityManager& em, ClanRepository& cr, EventPublisher& ep);

    bool areClanmates(uint32_t dbId1, uint32_t dbId2) const;
    int countNearbyClanmates(uint32_t dbId, int range) const;
    void notifyClanOfAttack(uint32_t targetDbId, const std::string& attackerName) const;

    // Calcula los modificadores de clan para un par atacante/target,
    // y notifica a los miembros del clan del objetivo si es necesario.
    CombatModifiers buildModifiers(uint32_t attackerDbId, const Attackable* target) const;
};

#endif  // CLAN_BONUS_CALCULATOR_H
