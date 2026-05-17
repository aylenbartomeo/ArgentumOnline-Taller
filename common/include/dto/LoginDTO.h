#ifndef LOGINDTO_H
#define LOGINDTO_H

#include <string>
#include <utility>

struct LoginDTO {
    std::string username;
    std::string password;

    LoginDTO() = default;

    LoginDTO(const std::string& username, const std::string& password):
            username(std::move(username)), password(std::move(password)) {}
};


#endif
