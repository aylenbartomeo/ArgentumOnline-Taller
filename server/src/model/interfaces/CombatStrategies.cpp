#include "CombatStrategies.h"
#include "../entities/Player.h"
#include "Attackable.h"
#include "../items/Weapon.h"
#include "../systems/CombatSystem.h"

bool InstantMeleeDelivery::deliver(Player& attacker, Attackable& target, 
                                   const CombatModifiers& modifiers, 
                                   const Weapon& weapon, 
                                   CombatSystem& combatSystem) {
    // 1. Validación de rango cuerpo a cuerpo (Fijo a lo pedido: rango de la Weapon, que suele ser 1 tile)
    if (attacker.distance_to(target) > weapon.getAttackRange()) {
        return false; // El ataque no ocurrió (fuera de rango)
    }

    // 2. Validación de estado
    if (target.isDead() || !target.canBeAttacked()) {
        return false;
    }

    // Como es cuerpo a cuerpo inmediato, el impacto se ejecuta en este mismo frame/tick
    if (weapon.getHitEffect()) {
        weapon.getHitEffect()->apply(attacker, target, modifiers, weapon, combatSystem);
    }
    return true;
}

bool ProjectileDelivery::deliver(Player& attacker, Attackable& target, 
                                 const CombatModifiers& modifiers, 
                                 const Weapon& weapon, 
                                 CombatSystem& combatSystem) {
    // 1. En armas a distancia, la distancia máxima la da el arma, pero no se resuelve el daño acá.
    if (attacker.distance_to(target) > weapon.getAttackRange()) {
        return false; 
    }

    // 2. Validar línea de visión inicial mediante el mapa (mencionado en la guía PDF)
    // Supongamos que combatSystem expone el acceso al mapa o validación de visión:
    // if (!combatSystem.hasLineOfSight(attacker, target)) return false;

    // 3. ¡EL COUPLING DE RED Y FÍSICA! 
    // Invocamos al sistema de tu compañero. Él necesitará saber quién dispara, 
    // a quién, y qué EFECTO se ejecutará al impactar.
    // Pasamos 'weapon.getHitEffect()' para que el proyectil lo arrastre en su viaje.
    
    // NOTA: Adaptá esta línea a la firma exacta que haya creado tu compañero:
    // projectileSystem.spawnProjectile(&attacker, &target, weapon.getHitEffect(), modifiers);

    return true; 
}

void MeleeDamageEffect::apply(Attackable& attacker, Attackable& target, 
                              const CombatModifiers& modifiers, 
                              const Weapon& weapon, 
                              CombatSystem& combatSystem) {
    
    // Encapsulamos los parámetros que solían viajar en AttackParams
    AttackParams params{
        static_cast<uint16_t>(weapon.getMinDamage()),
        static_cast<uint16_t>(weapon.getMaxDamage()),
        weapon.getAttackRange(),
        0,      // manaCost (Cuerpo a cuerpo físico no consume maná)
        false,  // isMagic
        modifiers.attackBonus,
        modifiers.defenseBonus
    };

    // Reutilizamos el método público aislado que creamos en la Fase 1
    combatSystem.applyDamageEffect(attacker, target, params);
}

void MagicDamageEffect::apply(Attackable& attacker, Attackable& target, 
                              const CombatModifiers& modifiers, 
                              const Weapon& weapon, 
                              CombatSystem& combatSystem) {
    
    // Tratamos de castear el atacante a Player porque los monstruos no suelen usar el sistema de maná de ítems
    // Si tu arquitectura permite que los monstruos usen Weapon, esto se puede abstraer a Attackable.
    Player* playerAttacker = dynamic_cast<Player*>(&attacker);
    
    if (playerAttacker) {
        int manaCost = weapon.getManaCost();
        if (!playerAttacker->consumeMana(manaCost)) {
            return; 
        }
    }

    // Si pasó el filtro de maná, ejecutamos el daño mágico
    AttackParams params{
        static_cast<uint16_t>(weapon.getMinDamage()),
        static_cast<uint16_t>(weapon.getMaxDamage()),
        weapon.getAttackRange(),
        weapon.getManaCost(),
        true,  // isMagic = true
        modifiers.attackBonus,
        modifiers.defenseBonus
    };

    combatSystem.applyDamageEffect(attacker, target, params);
}
