#include "EventPublisher.h"

#include <utility>

void EventPublisher::sendTo(uint32_t dbId, const std::string& message) {
    events.push_back({EventType::PRIVATE, dbId, message});
}

void EventPublisher::broadcast(const std::string& message) {
    events.push_back({EventType::BROADCAST, 0, message});
}

std::vector<WorldEvent> EventPublisher::pollEvents() {
    std::vector<WorldEvent> result = std::move(events);
    events.clear();
    return result;
}
