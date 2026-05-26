#include <filesystem>

#include <gtest/gtest.h>

#include "../server/src/auth/AuthManager.h"

namespace fs = std::filesystem;

class AuthManagerTest: public ::testing::Test {
protected:
    // Usamos una carpeta distinta para no borrar tus usuarios de verdad
    std::string testDir = "test_auth_data/";

    void SetUp() override {
        // Antes de cada test, nos aseguramos de que la carpeta esté limpia
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
        fs::create_directories(testDir);
    }

    void TearDown() override {
        // Al terminar el test, borramos la evidencia
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }
};

TEST_F(AuthManagerTest, RegisterNewUserGetsIdOne) {
    AuthManager auth(testDir);
    auto id = auth.registerUser("Franco", "1234");

    EXPECT_TRUE(id.has_value());
    EXPECT_EQ(id.value(), 1);
    EXPECT_TRUE(fs::exists(testDir + "Franco.toml"));
}

TEST_F(AuthManagerTest, CannotRegisterDuplicateUser) {
    AuthManager auth(testDir);
    auth.registerUser("Franco", "1234");

    auto duplicateId = auth.registerUser("Franco", "9999");  // Intento de robo
    EXPECT_FALSE(duplicateId.has_value());
}

TEST_F(AuthManagerTest, ValidationSucceedsWithCorrectPassword) {
    AuthManager auth(testDir);
    auth.registerUser("Cami", "claveSegura");

    auto validId = auth.validateUser("Cami", "claveSegura");
    EXPECT_TRUE(validId.has_value());
    EXPECT_EQ(validId.value(), 1);
}

TEST_F(AuthManagerTest, ValidationFailsWithWrongPasswordOrUnknownUser) {
    AuthManager auth(testDir);
    auth.registerUser("Cami", "claveSegura");

    auto wrongPass = auth.validateUser("Cami", "0000");
    EXPECT_FALSE(wrongPass.has_value());

    auto unknownUser = auth.validateUser("Fantasma", "claveSegura");
    EXPECT_FALSE(unknownUser.has_value());
}

TEST_F(AuthManagerTest, IDPersistsAndIncrementsAcrossRestarts) {
    // Simular primera ejecución del servidor
    {
        AuthManager auth1(testDir);
        auth1.registerUser("Franco", "1234");  // ID 1
        auth1.registerUser("Cami", "5678");    // ID 2
    }  // auth1 se destruye acá al salir de las llaves, simulando apagado

    // Simular reinicio del servidor
    {
        AuthManager auth2(testDir);  // Vuelve a escanear el disco
        auto newId = auth2.registerUser("Fede", "abcd");

        EXPECT_TRUE(newId.has_value());
        EXPECT_EQ(newId.value(), 3);  // Fede tiene que ser el 3

        auto francoId = auth2.validateUser("Franco", "1234");
        EXPECT_TRUE(francoId.has_value());
        EXPECT_EQ(francoId.value(), 1);  // Franco tiene que seguir siendo el 1
    }
}
