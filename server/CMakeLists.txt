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
    src/model/FormulaEngine.cpp

    # Entities
    src/model/entities/Player.cpp
    src/model/entities/Monster.cpp
    src/model/entities/MonsterFactory.cpp
    src/model/entities/Merchant.cpp

    # Items
    src/model/items/Armor.cpp
    src/model/items/ArmorFactory.cpp
    src/model/items/BodyArmor.cpp
    src/model/items/Helmet.cpp
    src/model/items/Shield.cpp
    src/model/items/Weapon.cpp  
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
    src/Map.cpp
    src/World.cpp

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

    src/model/items/Armor.h
    src/model/items/ArmorFactory.h
    src/model/items/BodyArmor.h
    src/model/items/Helmet.h
    src/model/items/Item.h
    src/model/items/ItemRegistry.h
    src/model/items/Shield.h
    src/model/items/Weapon.h
    src/model/items/WeaponFactory.h
    
    src/Acceptor.h
    src/ClientHandler.h
    src/Receiver.h
    src/Sender.h
    src/Server.h
    src/ConnectionMonitor.h
    src/GameLoop.h
    include/model/FormulaEngine.h

    src/Map.h
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
    src/model/items
    src/model/components
    ${CMAKE_SOURCE_DIR}/common/utils
)

target_link_libraries(argentum_online_server tomlplusplus::tomlplusplus pthread bcrypt)
target_include_directories(argentum_online_server PRIVATE ${libbcrypt_SOURCE_DIR}/include)
