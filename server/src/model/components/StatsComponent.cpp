#include "StatsComponent.h"

StatsComponent::StatsComponent(Race raceEnum, CharacterClass classEnum,
                               const RaceConfig& raceConfigData,
                               const CharacterClassConfig& characterClassData,
                               const PlayerConfig& playerBase, const FormulaEngine& engine):
        formulaEngine(engine),
        raceConfig(raceConfigData),
        classConfig(characterClassData),
        strength(static_cast<uint8_t>(playerBase.baseStrength)),
        intelligence(static_cast<uint8_t>(playerBase.baseIntelligence)),
        agility(static_cast<uint8_t>(playerBase.baseAgility)),
        constitution(static_cast<uint8_t>(playerBase.baseConstitution)),
        exp(playerBase.startingExperience),
        level(playerBase.startingLevel),
        race(raceEnum),
        characterClass(classEnum) {
    // Calculamos los techos iniciales
    recalculateMaxStats();

    // Tanques llenos al iniciar
    this->health = this->max_health;
    this->mana = this->max_mana;
}

StatsComponent::StatsComponent(const RaceConfig& raceConfigData,
                               const CharacterClassConfig& characterClassData,
                               const PlayerConfig& playerBase, const FormulaEngine& engine):
        formulaEngine(engine),
        raceConfig(raceConfigData),
        classConfig(characterClassData),
        strength(static_cast<uint8_t>(playerBase.baseStrength)),
        intelligence(static_cast<uint8_t>(playerBase.baseIntelligence)),
        agility(static_cast<uint8_t>(playerBase.baseAgility)),
        constitution(static_cast<uint8_t>(playerBase.baseConstitution)),
        exp(playerBase.startingExperience),
        level(playerBase.startingLevel),
        race(Race::HUMAN),
        characterClass(CharacterClass::WARRIOR) {
    recalculateMaxStats();
    this->health = this->max_health;
    this->mana = this->max_mana;
}

void StatsComponent::recalculateMaxStats() {
    this->max_health =
            formulaEngine.calculate_max_life(this->constitution, this->classConfig.lifeFactor,
                                             this->raceConfig.lifeFactor, this->level);

    this->max_mana =
            formulaEngine.calculate_max_mana(this->intelligence, this->classConfig.manaFactor,
                                             this->raceConfig.manaFactor, this->level);
}

void StatsComponent::addExperience(uint32_t amount) {
    exp += amount;
    while (exp >= formulaEngine.calculate_level_up_limit(level)) {
        level++;
        recalculateMaxStats();
        health = max_health;
        mana = max_mana;
    }
}

void StatsComponent::takeDamage(uint16_t amount) {
    health = (amount >= health) ? 0 : health - amount;
}

void StatsComponent::heal(uint16_t amount) {
    if (health == 0)
        return;
    health = std::min(static_cast<uint16_t>(health + amount), max_health);
}
void StatsComponent::restoreHp() { health = max_health; }

bool StatsComponent::consumeMana(uint16_t amount) {
    if (amount > mana)
        return false;
    mana -= amount;
    return true;
}

void StatsComponent::recoverMana(uint16_t amount) {
    if (health == 0)
        return;
    mana = std::min(static_cast<uint16_t>(mana + amount), max_mana);
}

void StatsComponent::loseExperienceUponDeath() {
    uint32_t loss = formulaEngine.calculate_death_exp_loss(this->level);

    uint32_t minExpAllowed =
            (this->level == 1) ? 0 : formulaEngine.calculate_level_up_limit(this->level - 1);

    if (this->exp > loss && (this->exp - loss) >= minExpAllowed) {
        this->exp -= loss;
    } else {
        this->exp = minExpAllowed;
    }
}

void StatsComponent::restoreMana() { mana = max_mana; }
