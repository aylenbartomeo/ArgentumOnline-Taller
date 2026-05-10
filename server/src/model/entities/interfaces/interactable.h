// server/src/model/interfaces/interactable.h
#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include <string>
#include <vector>

class Interactable {
public:
    virtual ~Interactable() = default;

    // El contrato de comunicación: quién interactúa, qué quiere hacer y qué datos extra envía.
    virtual void interact(Interactable& interactable, 
                          const std::string& action, 
                          const std::vector<std::string>& params) = 0;
};

#endif
