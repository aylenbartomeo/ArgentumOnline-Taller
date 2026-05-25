#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include <variant>
#include "Snapshot.h"
#include "ClientCommands.h" // For ChatDTO

using ServerMessageVariant = std::variant<SnapshotDTO, ChatDTO>;

#endif // SERVER_MESSAGE_H
