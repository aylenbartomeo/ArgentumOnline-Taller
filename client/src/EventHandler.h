#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

struct FrameInput {
    bool quit = false;
};

class EventHandler {
public:
    EventHandler() = default;
    ~EventHandler() = default;

    FrameInput poll_events();

    /* No permito copias */
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

    /* Permito movimientos */
    EventHandler(EventHandler&&) = default;
    EventHandler& operator=(EventHandler&&) = default;
};

#endif
