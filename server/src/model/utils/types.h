#ifndef TYPES_H
#define TYPES_H

enum class Race {
    HUMAN,
    ELF,
    DWARF,
    GNOME
};

enum class CharacterClass {
    MAGE,
    WARRIOR,
    PALADIN,
    CLERIC
};

// Tipos de NPCs disponibles en el juego
enum class NPCType {
    // --- CIUDADANOS ---
    MERCHANT,  // Comerciante: compra y vende items
    BANKER,    // Banquero: guarda oro e items
    PRIEST,    // Sacerdote: cura y resucita

    // --- CRIATURAS / MONSTRUOS ---
    GOBLIN,
    SKELETON,
    ZOMBIE,
    SPIDER,
    ORC,
    GOLEM
};

#endif
