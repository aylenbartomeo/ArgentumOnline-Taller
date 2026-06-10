#include "CombatStrategies.h"

#include "../entities/Player.h"
#include "../items/Weapon.h"
#include "../systems/CombatSystem.h"

#include "Attackable.h"

CombatResult InstantMeleeDelivery::deliver(Attackable& attacker, Attackable& target,
                                           const CombatModifiers& modifiers, const Weapon& weapon,
                                           CombatSystem& combatSystem) {
    // 1. Validación de rango cuerpo a cuerpo (Fijo a lo pedido: rango de la Weapon, que suele ser 1
    // tile)
    if (attacker.distance_to(target) > weapon.getAttackRange()) {
        return CombatResult{false};  // El ataque no ocurrió (fuera de rango)
    }

    // 2. Validación de estado
    if (target.isDead() || !target.canBeAttacked()) {
        return CombatResult{false};
    }

    // Como es cuerpo a cuerpo inmediato, el impacto se ejecuta en este mismo frame/tick
    if (weapon.getHitEffect()) {
        Player* playerAttacker = dynamic_cast<Player*>(&attacker);
        if (!playerAttacker) {
            return CombatResult{false};
        }
        return weapon.getHitEffect()->apply(*playerAttacker, target, modifiers, weapon, combatSystem);
    }
    return CombatResult{true};
}

CombatResult ProjectileDelivery::deliver(Attackable& attacker, Attackable& target,
                                         const CombatModifiers& modifiers, const Weapon& weapon,
                                         CombatSystem& combatSystem) {
    // 1. En armas a distancia, la distancia máxima la da el arma, pero no se resuelve el daño acá.
    if (attacker.distance_to(target) > weapon.getAttackRange()) {
        return CombatResult{false};
    }

    CombatResult res;
    res.attackHappened = true;
    res.isPending = true;
    return res;
}

CombatResult MeleeDamageEffect::apply(Player& attacker, Attackable& target,
                                      const CombatModifiers& modifiers, const Weapon& weapon,
                                      CombatSystem& combatSystem) {

    AttackParams params{static_cast<uint16_t>(weapon.getMinDamage()),
                        static_cast<uint16_t>(weapon.getMaxDamage()),
                        weapon.getAttackRange(),
                        0,      // manaCost (Cuerpo a cuerpo físico no consume maná)
                        false,  // isMagic
                        modifiers.attackBonus,
                        modifiers.defenseBonus};

    return combatSystem.applyDamageEffect(attacker, target, params);
}

CombatResult MagicDamageEffect::apply(Player& attacker, Attackable& target,
                                      const CombatModifiers& modifiers, const Weapon& weapon,
                                      CombatSystem& combatSystem) {

    int manaCost = weapon.getManaCost();
    if (!attacker.consumeMana(manaCost)) {
        return CombatResult{false};
    }

    // Si pasó el filtro de maná, ejecutamos el daño mágico
    AttackParams params{static_cast<uint16_t>(weapon.getMinDamage()),
                        static_cast<uint16_t>(weapon.getMaxDamage()),
                        weapon.getAttackRange(),
                        weapon.getManaCost(),
                        true,  // isMagic = true
                        modifiers.attackBonus,
                        modifiers.defenseBonus};

    return combatSystem.applyDamageEffect(attacker, target, params);
}

CombatResult MagicHealEffect::apply(Player& attacker, Attackable& target,
                                    const CombatModifiers& modifiers, const Weapon& weapon,
                                    CombatSystem& combatSystem) {

    int manaCost = weapon.getManaCost();
    if (!attacker.consumeMana(manaCost)) {
        return CombatResult{false};
    }

    Player* playerTarget = dynamic_cast<Player*>(&target);
    if (!playerTarget) {
        return CombatResult{false};
    }

    return combatSystem.applyHealEffect(*playerTarget);
}
