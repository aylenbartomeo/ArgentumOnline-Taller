#ifndef TYPES_H
#define TYPES_H

#include <string>

enum class Race { HUMAN, ELF, DWARF, GNOME };

enum class CharacterClass { MAGE, WARRIOR, PALADIN, CLERIC };

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
    GOLEM,

    // --- BOSSES DE MAZMORRA ---
    BOSS_BALROG,  // Balrog Infernal
    BOSS_TITAN,   // Titán de Piedra
    BOSS_COLOSO,  // Coloso de Magma
    BOSS_ARACNE   // Aracne Abismal
};

enum class ZoneType { FOREST, DESERT, NORMAL };

inline ZoneType zoneTypeFromString(const std::string& s) {
    if (s == "forest")
        return ZoneType::FOREST;
    if (s == "desert")
        return ZoneType::DESERT;
    return ZoneType::NORMAL;
}

inline bool isBossType(NPCType type) {
    return type == NPCType::BOSS_BALROG || type == NPCType::BOSS_TITAN ||
           type == NPCType::BOSS_COLOSO || type == NPCType::BOSS_ARACNE;
}

inline std::string getRaceName(Race r) {
    switch (r) {
        case Race::HUMAN:
            return "Humano";
        case Race::ELF:
            return "Elfo";
        case Race::DWARF:
            return "Enano";
        case Race::GNOME:
            return "Gnomo";
        default:
            return "Desconocida";
    }
}

inline std::string getClassName(CharacterClass c) {
    switch (c) {
        case CharacterClass::MAGE:
            return "Mago";
        case CharacterClass::WARRIOR:
            return "Guerrero";
        case CharacterClass::PALADIN:
            return "Paladín";
        case CharacterClass::CLERIC:
            return "Clérigo";
        default:
            return "Desconocida";
    }
}

#endif
