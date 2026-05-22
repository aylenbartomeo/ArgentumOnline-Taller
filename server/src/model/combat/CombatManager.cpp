#include "CombatManager.h"
#include "interfaces/Combatant.h"
#include "items/Weapon.h"

void CombatManager::executeAttack(Combatant& attacker, Combatant& target, Weapon* weapon) {
    if (attacker.isDead() || target.isDead()) return;

    // NOTA: Validación Fair Play
    // Debemos agregar getLevel() y isNewbie() a Combatant
    // if (!FormulaEngine::getInstance().isFairPlay(attacker, target)) { return; }

    // Cálculo de Evasión
    bool isEvaded = FormulaEngine::getInstance().is_attack_eluded(target.getAgility());

    if (isEvaded) {
        // Nota: El ataque falló, se puede emitir un EventBus::publish para que suene un sonido
        return;
    }

    // uint16_t rawDamage = 0;
    WeaponType type = (weapon != nullptr) ? weapon->getType() : WeaponType::MELEE;
    
    auto it = strategies.find(type);
    if (it != strategies.end()) {
        // rawDamage = it->second->calculateDamage(attacker, weapon); 
        return;
    }

    // Mitigación de Daño (NOTA: chequear esta logica para calcular el daño neto!!!)
    // uint16_t netDamage = FormulaEngine::getInstance().calculateNetDamage(rawDamage, target.getTotalDefense());

    // if (netDamage > 0) {
    //     target.receiveDamage(netDamage);
    // }
}