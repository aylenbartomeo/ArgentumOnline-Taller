#include "StatsComponent.h"

StatsComponent::StatsComponent(const RaceConfig& raceConf, const CharacterClassConfig& classConfig,
                               const PlayerConfig& playerBase, Race race, CharacterClass charClass,
                               const FormulaEngine& engine):
        formulaEngine(engine),
        raceConfig(raceConf),
        classConfig(classConfig),
        race(race),
        charClass(charClass),
        strength(static_cast<uint8_t>(playerBase.baseStrength)),
        intelligence(static_cast<uint8_t>(playerBase.baseIntelligence)),
        agility(static_cast<uint8_t>(playerBase.baseAgility)),
        constitution(static_cast<uint8_t>(playerBase.baseConstitution)),
        exp(playerBase.startingExperience),
        level(playerBase.startingLevel) {
    // Calculamos los techos iniciales
    recalculateMaxStats();

    // Tanques llenos al iniciar
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
        health = max_health;  // Full restore al subir de nivel
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
