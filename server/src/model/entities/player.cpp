#include "player.h"

Player::Player(uint32_t id, const std::string & name, Race race, CharacterClass char_class, Position pos)
{
}

void Player::receive_damage(int amount) {
    // Lógica de mitigación por agilidad/armadura segun clase y equipo
    this->health -= amount; 
}

void Player::attack(Combatant& target) {
    // Calcular daño según fuerza y arma
    int damage; // Aca habria que agregar la logica para calcular el daño segun la clase, el arma equipada, etc. 
                // Con FormulaEngine
    target.receive_damage(damage);
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
