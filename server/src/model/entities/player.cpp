#include "player.h"
#include "../combat/CombatManager.h"

#include <algorithm>

Player::Player(uint32_t id, const std::string & name, Race race, CharacterClass char_class, Position pos,
               FormulaEngine& formulas, CombatManager& combat_manager, const PlayerConfig& playerConfig,
               const RaceConfig& raceConfig, const CharacterClassConfig& classConfig)
    : id(id),
      name(name),
      race(race),
      char_class(char_class),
      pos(pos),
      equipment(),
      strength(playerConfig.baseStrength),
      intelligence(playerConfig.baseIntelligence),
      agility(playerConfig.baseAgility),
      constitution(playerConfig.baseConstitution),
      health(0),
      max_health(0),
      mana(0),
      max_mana(0),
      gold(playerConfig.startingGold),
      max_gold(0),
      experience(playerConfig.startingExperience),
      level(playerConfig.startingLevel),
      formulas(formulas),
      combat_manager(combat_manager),
      can_use_magic(classConfig.canUseMagic),
      can_meditate(classConfig.canUseMagic && classConfig.meditationFactor > 0.0f),
      recovery_factor(raceConfig.recoveryFactor),
      meditation_factor(classConfig.meditationFactor),
      state(PlayerState::Alive)
{
    this->max_health = this->formulas.calculate_max_life(
            static_cast<uint16_t>(this->constitution), classConfig.lifeFactor, raceConfig.lifeFactor,
            this->level);
    this->health = this->max_health;

    this->max_mana = this->can_use_magic ?
                             this->formulas.calculate_max_mana(
                                     static_cast<uint16_t>(this->intelligence), classConfig.manaFactor,
                                     raceConfig.manaFactor, this->level) :
                             0;
    this->mana = this->max_mana;

    this->max_gold = this->formulas.calculate_safe_gold_limit(this->level);
}

//IMPORTANTE: Habría que considerar que en vez de pedirle el daño a las armas o defensa al equipamiento del jugador
// podríamos delegar el cálculo del daño o defensa al objeto, por ejemplo: this->equipment.generate_attack_damage(this->strength, this->formulas)
// y similar con la defensa

void Player::receive_damage(int amount) {
    if (this->isGhost()) {
        return;
    }

    this->stopMeditating();

    // 1. Probabilidad de Evadir
    if (this->formulas.is_attack_eluded(static_cast<uint16_t>(this->agility))) {
        // El ataque falló, no restamos nada
        return; 
    }

    // 2. Mitigación por defensa equipada.
    int def = this->equipment.getDefense();
    
    // 3. Aplicar daño final
    int final_damage = std::max(0, amount - static_cast<int>(def));
    this->health -= final_damage;

    if (this->health <= 0) {
        this->becomeGhost();
    }
}

void Player::attack(Combatant& target) {
    if (this->isGhost()) {
        return;
    }

    this->stopMeditating();

    Weapon* equipped_weapon = this->equipment.getWeapon();
    if (equipped_weapon == nullptr) {
        return; 
    }

    // Ejecutar el ataque
    bool attack_success = this->combat_manager.executeAttack(
        *equipped_weapon, 
        *this,
        target,
        this->formulas
    );

    // Si el ataque fue exitoso y el objetivo muere, damos la XP
    if (attack_success && target.is_dead()) {
        // Lógica de ganar experiencia...
    }    
}

bool Player::is_dead() const { return health <= 0; }

Position Player::get_position() const { return this->pos; }

PlayerState Player::getState() const { return this->state; }

bool Player::isMeditating() const { return this->state == PlayerState::Meditating; }

bool Player::isGhost() const { return this->state == PlayerState::Ghost; }

bool Player::startMeditating() {
    if (this->state != PlayerState::Alive || !this->can_meditate || this->mana >= this->max_mana) {
        return false;
    }

    this->state = PlayerState::Meditating;
    return true;
}

void Player::stopMeditating() {
    if (this->state == PlayerState::Meditating) {
        this->state = PlayerState::Alive;
    }
}

void Player::recoverOverTime(float secondsElapsed) {
    if (this->isGhost() || secondsElapsed <= 0.0f) {
        return;
    }

    const uint16_t recoveredHealth =
            this->formulas.calculate_passive_recovery(this->recovery_factor, secondsElapsed);
    this->recoverHealth(static_cast<int>(recoveredHealth));

    if (!this->isMeditating() && this->can_use_magic) {
        const uint16_t recoveredMana =
                this->formulas.calculate_passive_recovery(this->recovery_factor, secondsElapsed);
        this->recoverMana(static_cast<int>(recoveredMana));
    }
}

void Player::recoverMeditating(float secondsElapsed) {
    if (!this->isMeditating() || secondsElapsed <= 0.0f) {
        return;
    }

    const uint16_t recoveredMana = this->formulas.calculate_meditation_recovery(
            this->meditation_factor, static_cast<uint16_t>(this->intelligence), secondsElapsed);
    this->recoverMana(static_cast<int>(recoveredMana));

    if (this->mana >= this->max_mana) {
        this->stopMeditating();
    }
}

void Player::interact(Interactable &interactable, const std::string &action, const std::vector<std::string> &params)
{
    this->stopMeditating();

    (void)interactable;
    (void)action;
    (void)params;
    // Aca deberia ocurrir la interaccion con el NPC de ciudad, dependiendo del tipo de NPC y la accion, 
    // se llamaria a los metodos correspondientes (buy, sell, heal, respawn, etc).
    // Habria que generar la comunicacion dado que ambos son Interactables
}

void Player::buy(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de compra con el comerciante
}

void Player::sell(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de venta con el comerciante
}

void Player::respawn()
{
    // Implementar logica de respawn en el punto de inicio o ultimo checkpoint
}

void Player::heal()
{
    // Implementar logica de curacion con el sacerdote
}

void Player::deposit_object(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de deposito de objetos en el banco
}

void Player::withdraw_object(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de retiro de objetos del banco
}

void Player::deposit_gold(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de deposito de oro en el banco
}

void Player::withdraw_gold(const std::vector<std::string> &params)
{
    (void)params;
    // Implementar logica de retiro de oro del banco
}

uint16_t Player::get_strength() const { return this->strength; }
uint16_t Player::get_intelligence() const { return this->intelligence; }
int Player::get_mana() const { return this->mana; }
void Player::consume_mana(int amount) { this->mana = std::max(0, this->mana - amount); }
bool Player::canUseMagic() const { return this->can_use_magic; }
bool Player::canMeditate() const { return this->can_meditate; }

Equipment& Player::getEquipment() { return this->equipment; }

const Equipment& Player::getEquipment() const { return this->equipment; }

void Player::recoverHealth(int amount) {
    if (amount <= 0) {
        return;
    }

    this->health = std::min(this->max_health, this->health + amount);
}

void Player::recoverMana(int amount) {
    if (amount <= 0 || !this->can_use_magic) {
        return;
    }

    this->mana = std::min(this->max_mana, this->mana + amount);
}

void Player::becomeGhost() {
    this->health = 0;
    this->mana = 0;
    this->state = PlayerState::Ghost;
}
