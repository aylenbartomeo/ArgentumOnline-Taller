#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

class EventHandler {
private:

public:
    EventHandler();

    /* No permito copias */
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

    /* Permito movimientos*/
    EventHandler(EventHandler&&) = default;
    EventHandler& operator=(EventHandler&&) = default;

    ~EventHandler();
};

#endif
