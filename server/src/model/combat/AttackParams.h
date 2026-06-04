#ifndef ATTACK_PARAMS_H
#define ATTACK_PARAMS_H

#include <cstdint>

// Struct intermediario que desacopla la estrategia de combate
// de la fuente de los parámetros (Weapon para un Player, config para un Monster).
struct AttackParams {
    uint16_t minDamage;
    uint16_t maxDamage;
    int attackRange;
    int manaCost;               // 0 para ataques físicos y monstruos
    bool isMagic;               // true para ataques mágicos (determina la estrategia)
    float attackBonus = 1.0f;   // multiplicador de ataque por bonus grupal de clan
    float defenseBonus = 1.0f;  // multiplicador de defensa por bonus grupal de clan
};

#endif
