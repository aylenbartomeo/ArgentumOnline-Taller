#ifndef REGISTERDTO_H
#define REGISTERDTO_H

#include <string>
#include <utility>

struct RegisterDTO {
    std::string username;
    std::string password;

    RegisterDTO() = default;

    RegisterDTO(const std::string& username, const std::string& password):
            username(std::move(username)), password(std::move(password)) {}
};

#endif
