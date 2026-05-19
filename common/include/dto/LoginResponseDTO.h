#ifndef LOGIN_RESPONSE_DTO_H
#define LOGIN_RESPONSE_DTO_H

#include <cstdint>
#include <string>

struct LoginResponseDTO {
    bool success;
    uint32_t clientId;
    std::string errorMessage;
};

#endif
