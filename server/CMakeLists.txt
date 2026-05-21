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
    src/model/entities/Citizen.cpp

    # Inventory
    src/model/inventory/inventory.cpp

    # Items
    src/model/items/Armor.cpp
    src/model/items/ArmorFactory.cpp
    src/model/items/BodyArmor.cpp
    src/model/items/Equipment.cpp
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

    # Game
    src/Map.cpp
    src/World.cpp

    # Components
    src/model/entities/components/InventoryComponent.cpp
    src/model/entities/components/EquipmentComponent.cpp
    src/model/entities/components/BankComponent.cpp

    PUBLIC
    # .h files
    src/config/ArmorConfigLoader.h
    src/config/CharacterConfig.h
    src/config/CharacterConfigLoader.h
    src/config/ItemConfigLoader.h
    src/config/MonsterConfig.h
    src/config/MonsterConfigLoader.h

    src/model/entities/MonsterFactory.h
    src/model/entities/PlayerMock.h
    src/model/entities/components/StatsComponent.h
    src/model/entities/components/InventoryComponent.h
    src/model/entities/components/EquipmentComponent.h
    src/model/entities/components/BankComponent.h

    src/model/inventory/inventory.h 
    src/model/items/Armor.h
    src/model/items/ArmorFactory.h
    src/model/items/BodyArmor.h
    src/model/items/Equipment.h
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
)

# Include directories
target_include_directories(argentum_online_server
    PRIVATE
    include
    src
    src/model
    src/model/entities
    src/model/items
    src/model/inventory # Agregado para que encuentre limpiamente las cabeceras
    src/model/entities/components
)

target_link_libraries(argentum_online_server tomlplusplus::tomlplusplus pthread)
