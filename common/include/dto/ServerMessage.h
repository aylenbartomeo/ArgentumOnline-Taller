#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include <variant>

#include "ClientCommands.h"  // For ChatDTO
#include "PlayerStatsDTO.h"
#include "Snapshot.h"

using ServerMessageVariant = std::variant<SnapshotDTO, ChatDTO, PlayerStatsDTO>;

#endif  // SERVER_MESSAGE_H
