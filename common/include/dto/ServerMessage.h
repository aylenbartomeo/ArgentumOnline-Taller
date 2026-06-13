#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include <variant>

#include "ClientCommands.h"  // For ChatDTO
#include "JoinResponseDTO.h"
#include "PlayerStatsDTO.h"
#include "Snapshot.h"

using ServerMessageVariant = std::variant<SnapshotDTO, ChatDTO, PlayerStatsDTO, JoinResponseDTO>;

#endif  // SERVER_MESSAGE_H
