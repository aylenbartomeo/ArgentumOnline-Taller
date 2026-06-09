// model/items/CombatStrategies.h
#pragma once

#include <cstdint>

#include "../systems/CombatSystem.h"

// Forward declarations para evitar acoplamiento circular de headers
class Player;
class Attackable;
class Weapon;

// Estructura de modificadores contextuales que calculaba el World/CombatSystem
struct CombatModifiers {
    float attackBonus = 1.0f;
    float defenseBonus = 1.0f;
};

// ==========================================
// INTERFACES BASE
// ==========================================

// Estrategia de entrega: ¿Cómo viaja el ataque?
class IAttackDelivery {
public:
    virtual ~IAttackDelivery() = default;
    virtual CombatResult deliver(Player& attacker, Attackable& target,
                                 const CombatModifiers& modifiers, const Weapon& weapon,
                                 CombatSystem& combatSystem) = 0;
};

// Estrategia de impacto: ¿Qué pasa cuando conecta?
class IHitEffect {
public:
    virtual ~IHitEffect() = default;
    virtual CombatResult apply(Attackable& attacker, Attackable& target,
                               const CombatModifiers& modifiers, const Weapon& weapon,
                               CombatSystem& combatSystem) = 0;
};

// ==========================================
// DECLARACIÓN DE CLASES CONCRETAS
// ==========================================

// Implementación para ataques inmediatos / cuerpo a cuerpo
class InstantMeleeDelivery: public IAttackDelivery {
public:
    ~InstantMeleeDelivery() override = default;

    CombatResult deliver(Player& attacker, Attackable& target, const CombatModifiers& modifiers,
                         const Weapon& weapon, CombatSystem& combatSystem) override;
};

// Implementacion de ataques a travez de proyectiles
class ProjectileDelivery: public IAttackDelivery {
public:
    ~ProjectileDelivery() override = default;

    CombatResult deliver(Player& attacker, Attackable& target, const CombatModifiers& modifiers,
                         const Weapon& weapon, CombatSystem& combatSystem) override;
};

// Implementación para efectos de daño físico tradicional (Espadas, Hachas, Arcos iniciales)
class MeleeDamageEffect: public IHitEffect {
public:
    ~MeleeDamageEffect() override = default;

    CombatResult apply(Attackable& attacker, Attackable& target, const CombatModifiers& modifiers,
                       const Weapon& weapon, CombatSystem& combatSystem) override;
};

// Implementación para efectos de daño mágico con consumo de maná (Bastones)
class MagicDamageEffect: public IHitEffect {
public:
    ~MagicDamageEffect() override = default;

    CombatResult apply(Attackable& attacker, Attackable& target, const CombatModifiers& modifiers,
                       const Weapon& weapon, CombatSystem& combatSystem) override;
};
