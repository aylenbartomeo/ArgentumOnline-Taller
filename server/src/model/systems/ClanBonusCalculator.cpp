#include "server/src/model/systems/ClanBonusCalculator.h"

#include <string>

#include "server/src/model/entities/Player.h"

ClanBonusCalculator::ClanBonusCalculator(EntityManager& em, ClanRepository& cr, EventPublisher& ep,
                                         const ServerConfig& config):
        entityManager(em),
        clanRepo(cr),
        eventPublisher(ep),
        CLAN_BONUS_RANGE(config.clanBonusRange),
        CLAN_ATTACK_BONUS_PER_MEMBER(config.clanAttackBonusPerMember),
        CLAN_DEFENSE_BONUS_PER_MEMBER(config.clanDefenseBonusPerMember) {}

bool ClanBonusCalculator::areClanmates(uint32_t dbId1, uint32_t dbId2) const {
    if (dbId1 == dbId2)
        return true;
    auto clan1 = clanRepo.getClanIdOfPlayer(dbId1);
    auto clan2 = clanRepo.getClanIdOfPlayer(dbId2);
    return clan1 && clan2 && *clan1 == *clan2;
}

int ClanBonusCalculator::countNearbyClanmates(uint32_t dbId, int range) const {
    auto clanId = clanRepo.getClanIdOfPlayer(dbId);
    if (!clanId)
        return 0;

    const Clan* clan = clanRepo.getClanById(*clanId);
    if (!clan)
        return 0;

    auto posOpt = entityManager.getPlayerPosition(dbId);
    if (!posOpt)
        return 0;

    int count = 0;
    for (uint32_t memberId: clan->getMembers()) {
        if (memberId == dbId)
            continue;
        auto memberPosOpt = entityManager.getPlayerPosition(memberId);
        if (memberPosOpt && posOpt->chebyshev_distance_to(*memberPosOpt) <= range) {
            count++;
        }
    }
    return count;
}

void ClanBonusCalculator::notifyClanOfAttack(uint32_t targetDbId,
                                             const std::string& attackerName) const {
    auto clanIdOpt = clanRepo.getClanIdOfPlayer(targetDbId);
    if (!clanIdOpt)
        return;

    const Clan* clan = clanRepo.getClanById(*clanIdOpt);
    if (!clan)
        return;

    // Resolve the target's name
    Player* targetPlayer = entityManager.getPlayer(targetDbId);
    std::string targetName = targetPlayer ? targetPlayer->getName() : "Un compañero";

    std::string alertMsg =
            "[Clan] " + targetName + " está siendo atacado por " + attackerName + "!";
    for (uint32_t memberId: clan->getMembers()) {
        if (memberId != targetDbId) {
            eventPublisher.sendTo(memberId, alertMsg);
        }
    }
}

CombatModifiers ClanBonusCalculator::buildModifiers(uint32_t attackerDbId,
                                                    const Attackable* target) const {
    CombatModifiers m;
    m.attackBonus = 1.0f + (countNearbyClanmates(attackerDbId, CLAN_BONUS_RANGE) *
                            CLAN_ATTACK_BONUS_PER_MEMBER);

    if (const Player* tp = dynamic_cast<const Player*>(target)) {
        uint32_t targetDb = tp->getDbId();
        m.defenseBonus +=
                countNearbyClanmates(targetDb, CLAN_BONUS_RANGE) * CLAN_DEFENSE_BONUS_PER_MEMBER;

        // Notify clan
        Player* attacker = entityManager.getPlayer(attackerDbId);
        std::string attackerName = attacker ? attacker->getName() : "Alguien";
        notifyClanOfAttack(targetDb, attackerName);
    }
    return m;
}
