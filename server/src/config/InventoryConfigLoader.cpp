#include "server/src/config/InventoryConfigLoader.h"
#include "server/src/config/TomlConfigHelper.h"

namespace {

// Definimos el contexto específico para el sistema de inventario
constexpr std::string_view CTX = "inventory";

}  // namespace

InventoryConfig InventoryConfigLoader::loadInventoryConfig(
        const std::filesystem::path& configPath) {
    const toml::table config = TomlHelper::parseConfigFile(configPath, CTX);
    const toml::table& inventoryTable = TomlHelper::requiredTable(config, "inventory", CTX);
    return InventoryConfig{
            TomlHelper::requiredUInt8(inventoryTable, "max_slots", CTX),
            static_cast<uint32_t>(TomlHelper::requiredInt(inventoryTable, "gold", CTX)),
            static_cast<uint32_t>(TomlHelper::requiredInt(inventoryTable, "max_gold", CTX)),
            static_cast<uint32_t>(
                    TomlHelper::requiredInt(inventoryTable, "initial_safe_gold", CTX))};
}
