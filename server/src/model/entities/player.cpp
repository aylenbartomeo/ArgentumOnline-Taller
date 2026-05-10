#include "player.h"
#include "../combat/CombatManager.h"

Player::Player(uint32_t id, const std::string & name, Race race, CharacterClass char_class, Position pos, FormulaEngine& formulas)
    : id(id), name(name), race(race), char_class(char_class), pos(pos), formulas(formulas),
      level(1), experience(0), strength(15), intelligence(15), agility(15), constitution(15), gold(0)
{
    // VALORES DE PRUEBA (Luego vendrán del TOML)
    float race_f = 1.0f; 
    float class_f = 1.0f;
    this->equipped_weapon = nullptr;
    this->max_health = this->formulas.calculate_max_life(this->constitution, class_f, race_f, this->level);
    this->health = this->max_health;

    this->max_mana = this->formulas.calculate_max_mana(this->intelligence, class_f, race_f, this->level);
    this->mana = this->max_mana;

    this->max_gold = this->formulas.calculate_safe_gold_limit(this->level);
}

//IMPORTANTE: Habría que considerar que en vez de pedirle el daño a las armas o defensa al equipamiento del jugador
// podríamos delegar el cálculo del daño o defensa al objeto, por ejemplo: this->equipment.generate_attack_damage(this->strength, this->formulas)
// y similar con la defensa

void Player::receive_damage(int amount) {
    // 1. Probabilidad de Evadir
    if (this->formulas.is_attack_eluded(static_cast<uint16_t>(this->agility))) {
        // El ataque falló, no restamos nada
        return; 
    }

    // 2. Mitigación por defensa (Valores de prueba de armadura/escudo/casco, se debería chequear inventario del jugador)
    uint16_t def = this->formulas.calculate_defense(5, 10, 0, 0, 0, 0); // Solo armadura por ahora (min-max)
    
    // 3. Aplicar daño final
    int final_damage = std::max(0, amount - static_cast<int>(def));
    this->health -= final_damage;

    if (this->health < 0) this->health = 0;
}

void Player::attack(Combatant& target) {
    if (!this->equipped_weapon) {
        return; 
    }

    CombatManager combat_manager;
    // Ejecutar el ataque
    bool attack_success = combat_manager.executeAttack(
        *this->equipped_weapon, 
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

void Player::interact(Interactable &interactable, const std::string &action, const std::vector<std::string> &params)
{
    // Aca deberia ocurrir la interaccion con el NPC de ciudad, dependiendo del tipo de NPC y la accion, 
    // se llamaria a los metodos correspondientes (buy, sell, heal, respawn, etc).
    // Habria que generar la comunicacion dado que ambos son Interactables
}

void Player::buy(const std::vector<std::string> &params)
{
    // Implementar logica de compra con el comerciante
}

void Player::sell(const std::vector<std::string> &params)
{
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
    // Implementar logica de deposito de objetos en el banco
}

void Player::withdraw_object(const std::vector<std::string> &params)
{
    // Implementar logica de retiro de objetos del banco
}

void Player::deposit_gold(const std::vector<std::string> &params)
{
    // Implementar logica de deposito de oro en el banco
}

void Player::withdraw_gold(const std::vector<std::string> &params)
{
    // Implementar logica de retiro de oro del banco
}

uint16_t Player::get_strength() const { return this->strength; }
uint16_t Player::get_intelligence() const { return this->intelligence; }
int Player::get_mana() const { return this->mana; }
void Player::consume_mana(int amount) { this->mana = std::max(0, this->mana - amount); }