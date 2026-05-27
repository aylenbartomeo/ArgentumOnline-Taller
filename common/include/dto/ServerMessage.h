#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include <variant>

#include "ClientCommands.h"  // For ChatDTO
#include "Snapshot.h"

using ServerMessageVariant = std::variant<SnapshotDTO, ChatDTO>;

#endif  // SERVER_MESSAGE_H
