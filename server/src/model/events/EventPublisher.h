#ifndef EVENT_PUBLISHER_H
#define EVENT_PUBLISHER_H

#include <cstdint>
#include <string>
#include <vector>

enum class EventType { PRIVATE, BROADCAST };

struct WorldEvent {
    EventType type;
    uint32_t targetDbId;  // 0 si es BROADCAST
    std::string message;
};

class EventPublisher {
private:
    std::vector<WorldEvent> events;

public:
    void sendTo(uint32_t dbId, const std::string& message);
    void broadcast(const std::string& message);
    std::vector<WorldEvent> pollEvents();
};

#endif  // EVENT_PUBLISHER_H
