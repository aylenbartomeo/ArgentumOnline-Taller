#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <atomic>
#include <optional>
#include <string>

class AuthManager {
private:
    std::string usersDirectory;
    std::atomic<uint32_t> nextId;

    std::string getUserFilePath(const std::string& username) const;
    void initializeNextId();

public:
    explicit AuthManager(const std::string& usersDir = "auth_data/");

    bool userExists(const std::string& username) const;
    std::optional<uint32_t> registerUser(const std::string& username, const std::string& password);
    std::optional<uint32_t> validateUser(const std::string& username,
                                         const std::string& password) const;
};

#endif
