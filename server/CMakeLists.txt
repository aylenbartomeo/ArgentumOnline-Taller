include(FetchContent)

FetchContent_Declare(
    libbcrypt
    GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libbcrypt)

target_sources(argentum_online_server
    PRIVATE
    # .cpp files
    src/main.cpp

    # Config
    src/config/ArmorConfigLoader.cpp
    src/config/CharacterConfigLoader.cpp
    src/config/ItemConfigLoader.cpp
    src/config/MonsterConfigLoader.cpp

    # Formulas
    include/FormulaEngine.cpp

    # Entities
    src/model/entities/Player.cpp
    src/model/entities/Monster.cpp
    src/model/entities/MonsterFactory.cpp
    src/model/entities/Merchant.cpp
    src/model/entities/Priest.cpp
    src/model/entities/Banker.cpp

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

    # Network
    src/Acceptor.cpp
    src/ClientHandler.cpp
    src/Receiver.cpp
    src/Sender.cpp
    src/Server.cpp
    src/ConnectionMonitor.cpp
    src/GameLoop.cpp
    src/auth/AuthManager.cpp
    src/persistence/PlayerDataStore.cpp

    # Game
    src/model/map/GroundItemLayer.cpp
    src/model/map/SafeZoneLayer.cpp
    src/model/map/NPCLayer.cpp
    src/model/map/CollisionLayer.cpp
    src/Map.cpp
    src/World.cpp

    # Clan
    src/model/clan/ClanRepository.cpp
    src/model/clan/ClanService.cpp
    src/model/clan/ClanController.cpp

    # Components
    src/model/components/InventoryComponent.cpp
    src/model/components/EquipmentComponent.cpp
    src/model/components/StatsComponent.cpp
    src/model/combat/CombatManager.cpp
    src/model/components/RegenerationComponent.cpp


    PUBLIC
    # .h files
    src/config/ArmorConfigLoader.h
    src/config/CharacterConfig.h
    src/config/CharacterConfigLoader.h
    src/config/ItemConfigLoader.h
    src/config/MonsterConfig.h
    src/config/MonsterConfigLoader.h

    src/model/entities/MonsterFactory.h
    src/model/entities/Player.h
    src/model/entities/PlayerFactory.h
    src/model/entities/Priest.h
    src/model/entities/Merchant.h
    src/model/entities/Banker.h
    src/model/entities/GlobalBank.h
    src/model/components/StatsComponent.h
    src/model/components/InventoryComponent.h
    src/model/components/EquipmentComponent.h
    src/model/combat/CombatManager.h

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

    src/Map.h
    src/model/map/GroundItemLayer.h
    src/model/map/SafeZoneLayer.h
    src/model/map/NPCLayer.h
    src/model/map/CollisionLayer.h
    src/World.h
    src/auth/AuthManager.h
    src/persistence/PlayerDataStore.h
)

# Include directories
target_include_directories(argentum_online_server
    PRIVATE
    include
    src
    src/model
    src/model/entities
    src/model/map
    src/model/items
    src/model/components
    src/model/handlers
    src/model/clan
    ${CMAKE_SOURCE_DIR}/common/utils
)

target_link_libraries(argentum_online_server tomlplusplus::tomlplusplus pthread bcrypt)
target_include_directories(argentum_online_server PRIVATE ${libbcrypt_SOURCE_DIR}/include)