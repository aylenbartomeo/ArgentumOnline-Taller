include(FetchContent)

# --- Dependencia 1: bcrypt ---
FetchContent_Declare(
    libbcrypt
    GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libbcrypt)

# --- Dependencia 2: tomlplusplus ---
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    URL https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.4.0.zip
)
FetchContent_MakeAvailable(tomlplusplus)

add_library(lib_server STATIC)

target_sources(lib_server
    PRIVATE
    # Config
    src/config/ServerConfigLoader.cpp
    src/config/CharacterConfigLoader.cpp
    src/config/ItemConfigLoader.cpp
    src/config/MonsterConfigLoader.cpp
    src/config/InventoryConfigLoader.cpp

    # Formulas
    include/FormulaEngine.cpp
    include/LootResolver.cpp

    # Entities
    src/model/entities/Player.cpp
    src/model/entities/Monster.cpp
    src/model/entities/MonsterFactory.cpp
    src/model/entities/Merchant.cpp
    src/model/entities/Priest.cpp
    src/model/entities/Banker.cpp
    src/model/entities/EntityManager.cpp
    src/model/entities/Projectile.cpp
    src/model/systems/ProjectileSystem.cpp
    src/model/systems/SpawnSystem.cpp
    src/model/systems/ResurrectionService.cpp
    src/model/systems/InteractionService.cpp
    src/model/systems/CombatSystem.cpp
    src/model/systems/ClanBonusCalculator.cpp
    src/model/systems/CombatNotifier.cpp

    # Items
    src/model/items/Armor.cpp
    src/model/items/ArmorFactory.cpp
    src/model/items/BodyArmor.cpp
    src/model/items/Helmet.cpp
    src/model/items/Shield.cpp
    src/model/items/Weapon.cpp  
    src/model/items/ConsumableFactory.cpp
    src/model/items/WeaponFactory.cpp
    src/model/items/ItemRegistry.cpp 

    # Network & Infrastructure
    src/Acceptor.cpp
    src/ClientHandler.cpp
    src/Receiver.cpp
    src/Sender.cpp
    src/Server.cpp
    src/ConnectionMonitor.cpp
    src/GameLoop.cpp
    src/auth/AuthManager.cpp
    src/persistence/PlayerDataStore.cpp
    src/persistence/WorldDataStore.cpp

    # Game Core
    src/model/map/GroundItemLayer.cpp
    src/model/map/SafeZoneLayer.cpp
    src/model/map/NPCLayer.cpp
    src/model/map/CollisionLayer.cpp
    src/Map.cpp
    src/World.cpp
    src/model/events/EventPublisher.cpp

    # Clan
    src/model/clan/ClanRepository.cpp
    src/model/clan/ClanService.cpp
    src/model/clan/ClanController.cpp

    # Components
    src/model/components/InventoryComponent.cpp
    src/model/components/EquipmentComponent.cpp
    src/model/components/StatsComponent.cpp
    src/model/components/RegenerationComponent.cpp

    # interfaces
    src/model/interfaces/CombatStrategies.cpp

    PUBLIC
    # Headers para visualización en IDEs y exposición pública
    src/config/TomlConfigHelper.h
    src/config/ServerConfigLoader.h
    src/config/CharacterConfig.h
    src/config/CharacterConfigLoader.h
    src/config/ItemConfigLoader.h
    src/config/MonsterConfig.h
    src/config/MonsterConfigLoader.h
    src/config/InventoryConfig.h
    src/config/InventoryConfigLoader.h

    # interfaces
    src/model/interfaces/CombatStrategies.h
    src/model/entities/MonsterFactory.h
    src/model/entities/Player.h
    src/model/entities/Priest.h
    src/model/entities/Merchant.h
    src/model/entities/Banker.h
    src/model/entities/GlobalBank.h
    src/model/entities/EntityManager.h
    src/model/systems/SpawnSystem.h
    src/model/systems/ResurrectionService.h
    src/model/systems/InteractionService.h
    src/model/systems/CombatSystem.h
    src/model/systems/ClanBonusCalculator.h
    src/model/systems/CombatNotifier.h
    src/model/systems/CombatTypes.h
    src/model/components/StatsComponent.h
    src/model/components/InventoryComponent.h
    src/model/components/EquipmentComponent.h

    src/model/handlers/NpcCommandHandler.h
    src/model/handlers/BankDepositHandler.h
    src/model/handlers/BankWithdrawHandler.h
    src/model/handlers/HealHandler.h
    src/model/handlers/TradeHandler.h
    src/model/handlers/ResurrectHandler.h

    src/model/items/Armor.h
    src/model/items/ArmorFactory.h
    src/model/items/BodyArmor.h
    src/model/items/Helmet.h
    src/model/items/Item.h
    src/model/items/ItemRegistry.h
    src/model/items/Shield.h
    src/model/items/Consumable.h
    src/model/items/ConsumableFactory.h
    src/model/items/Weapon.h
    src/model/items/WeaponFactory.h
    
    src/Acceptor.h
    src/ClientHandler.h
    src/Receiver.h
    src/Sender.h
    src/Server.h
    src/ConnectionMonitor.h
    src/GameLoop.h
    include/FormulaEngine.h
    include/LootResolver.h

    src/Map.h
    src/model/map/GroundItemLayer.h
    src/model/map/SafeZoneLayer.h
    src/model/map/NPCLayer.h
    src/model/map/CollisionLayer.h
    src/World.h
    src/auth/AuthManager.h
    src/persistence/PlayerDataStore.h
    src/persistence/WorldDataStore.h
    src/persistence/WorldPersistData.h
)

# Definimos las rutas de headers como INTERFACE/PUBLIC para que quien use la lib las herede
target_include_directories(lib_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/map
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/items
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/components
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/handlers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/clan
    ${CMAKE_SOURCE_DIR}/common/utils
    PRIVATE
    ${libbcrypt_SOURCE_DIR}/include
)

# Las dependencias ahora se vinculan directamente a la librería
target_link_libraries(lib_server PUBLIC 
    lib_common 
    tomlplusplus::tomlplusplus 
    pthread 
    bcrypt
    nlohmann_json::nlohmann_json
)

add_executable(argentum_online_server src/main.cpp)

set_target_properties(argentum_online_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
# El ejecutable solo necesita linkear contra nuestra nueva librería
target_link_libraries(argentum_online_server PRIVATE lib_server)

# Aplicar los warnings definidos en la raíz
set_project_warnings(argentum_online_server ${ARGENTUM_ONLINE_MAKE_WARNINGS_AS_ERRORS} FALSE)