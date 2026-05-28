#ifndef CLAN_MANAGER_H
#define CLAN_MANAGER_H

#include <cstdint>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <optional>

static constexpr uint16_t CLAN_MIN_LEVEL_TO_FOUND   = 6;
static constexpr uint16_t CLAN_MAX_MEMBERS           = 16;
static constexpr int      CLAN_BONUS_RANGE           = 10;  // casillas para considerar "cerca"
static constexpr float    CLAN_DEFENSE_BONUS_PER_MEMBER = 0.05f; // +5 % defensa por miembro cercano
static constexpr float    CLAN_ATTACK_BONUS_PER_MEMBER  = 0.05f;


enum class ClanOpResult {
    OK,
    CLAN_NOT_FOUND,
    PLAYER_NOT_FOUND,
    ALREADY_IN_CLAN,
    NOT_IN_CLAN,
    NOT_FOUNDER,
    LEVEL_TOO_LOW,
    NAME_TAKEN,
    CLAN_FULL,
    PLAYER_BANNED,
    NO_PENDING_REQUEST,
    CANNOT_LEAVE_AS_FOUNDER,
    SAME_CLAN,           // intento de atacar a un clanmate
};

// Estructura interna de un clan
struct Clan {
    uint32_t    id;
    std::string name;
    uint32_t    founderDbId;
 
    // miembros activos (incluye al fundador)
    std::unordered_set<uint32_t> members;
 
    // solicitudes pendientes (dbId del solicitante)
    std::unordered_set<uint32_t> pendingRequests;
 
    // jugadores baneados (sus futuras peticiones se auto-rechazan)
    std::unordered_set<uint32_t> banned;
};

// Mensaje que el ClanManager emite para que World lo reenvíe a los clientes
struct ClanNotification {
    uint32_t    targetDbId;
    std::string message;
};

class ClanManager {
private:
    uint32_t nextClanId = 1;
 
    // clanId → Clan
    std::unordered_map<uint32_t, Clan> clans;
 
    // Índice: nombre de clan (lower-case) → clanId  (para buscar por nombre en O(1))
    std::unordered_map<std::string, uint32_t> nameIndex;
 
    // Índice: playerDbId → clanId  (para saber a qué clan pertenece un jugador)
    std::unordered_map<uint32_t, uint32_t> playerClanIndex;
 
    static std::string toLower(const std::string& s);
 
    // Busca el clan del jugador y lo devuelve como puntero mutable, o nullptr
    Clan* getClanOfPlayer(uint32_t playerDbId);
    const Clan* getClanOfPlayer(uint32_t playerDbId) const;
 
    // Busca un clan por nombre (insensible a mayúsculas).
    Clan* findClanByName(const std::string& name);
 
    // Genera mensajes a todos los miembros de un clan excepto al excluido. */
    void broadcastToClan(const Clan& clan, const std::string& message, uint32_t excludeDbId,
                        std::vector<ClanNotification>& outNotifs) const;

public:
    ClanManager() = default;
    ~ClanManager() = default;

    ClanManager(const ClanManager&) = delete;
    ClanManager& operator = (const ClanManager&) = delete;


    // =======================================================================
    // Operaciones de clan
    // =======================================================================

    // Funda un clan nuevo
    ClanOpResult foundClan(uint32_t founderDbId, uint16_t founderLevel, const std::string& clanName, 
                        std::vector<ClanNotification>& notification);

    // Solicita unirsea un clan existente
    ClanOpResult joinRequest(uint32_t playerDbId, const std::string& clanName, 
                        std::vector<ClanNotification>& notification);

    // El fundador acepta a un nuevo miembro
    ClanOpResult acceptMember(uint32_t playerDbId, const std::string &targetNick, uint32_t targetDbId,
                        std::vector<ClanNotification>& notification);

    // El fundador rechaza a un nuevo miembro
    ClanOpResult rejectMember(uint32_t playerDbId, const std::string& targetNick, uint32_t targetDbId,
                        std::vector<ClanNotification>& notification);


    // El fundador bannea a un miembro
    ClanOpResult banMember(uint32_t playerDbId, const std::string& targetNick, uint32_t targetDbId,
                        std::vector<ClanNotification>& notification);

    // El fundador expulsa un miembro activo
    ClanOpResult kickMember(uint32_t playerDbId, const std::string& targetNick, uint32_t targetDbId,
                        std::vector<ClanNotification>& notification);

    // Un miembro abandona el clan por su propia voluntad
    ClanOpResult leaveClan(uint32_t playerDbId, std::vector<ClanNotification>& notification);

    // Devuelve un string con los miembros y solicitudes pendientes de un clan. Solo usable por el fundador
    ClanOpResult reviewClan(uint32_t founderDbId, std::string& outReport);

    // Devuelve el ID del clan al que pertenece el jugador, o nullopt
    std::optional<uint32_t> getClanId(uint32_t playerDbId) const;
 
    // Verifica si ambos jugadores pertenecen al mismo clan.
    bool areClanmates(uint32_t playerADbId, uint32_t playerBDbId) const;
 
    /**
     * Cuenta cuántos dbIds del vector están "cerca" (distancia Manhattan) de
     * una posición de referencia.  Usado para calcular el bonus grupal.
     * La posición se pasa como par (x, y) para no acoplar ClanManager con
     * la clase Position de otro módulo.
     */
    int countNearbyClanmates(uint32_t playerDbId,
                             const std::unordered_map<uint32_t, std::pair<int,int>>& memberPositions,
                             int refX, int refY, int range) const;

    // =======================================================================
    // Notificaciones de login/logoff
    // =======================================================================

    // Genera notificaciones a todos los clanmates del jugador indicando que entro/salio.
    void notifyLogin (uint32_t playerDbId, const std::string& username,
                    std::vector<ClanNotification>& outNotifs) const;

    void notifyLogoff(uint32_t playerDbId, const std::string& username,
                    std::vector<ClanNotification>& outNotifs) const;

    /**
     * Genera notificaciones a los clanmates (que NO son el atacado) indicando
     * que un miembro del clan está siendo atacado.
     */
    void notifyUnderAttack(uint32_t attackedDbId, const std::string& attackedName,
                           std::vector<ClanNotification>& outNotifs) const;

};

#endif
