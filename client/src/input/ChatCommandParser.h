#ifndef CHAT_COMMAND_PARSER_H
#define CHAT_COMMAND_PARSER_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "common/include/dto/ClientCommands.h"

class ChatCommandParser {
public:
    ChatCommandParser();

    // Parsea el texto del chat y devuelve el comando adecuado (o nullopt si es inválido)
    std::optional<CommandVariant> parse(const std::string& text) const;

private:
    using CommandHandler = std::function<CommandVariant(const std::string& args)>;

    std::unordered_map<std::string, CommandHandler> handlers;

    void registerHandlers();
};

#endif  // CHAT_COMMAND_PARSER_H
