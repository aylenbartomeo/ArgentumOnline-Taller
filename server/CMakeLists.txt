target_sources(taller_server
    PRIVATE
    # .cpp files
    main.cpp
    src/config/ArmorConfigLoader.cpp
    src/model/items/Armor.cpp
    src/model/items/ArmorFactory.cpp
    src/model/items/BodyArmor.cpp
    src/model/items/Helmet.cpp
    src/model/items/Shield.cpp
    PUBLIC
    # .h files
    src/config/ArmorConfigLoader.h
    src/model/items/Armor.h
    src/model/items/ArmorFactory.h
    src/model/items/BodyArmor.h
    src/model/items/Helmet.h
    src/model/items/Item.h
    src/model/items/Shield.h
    )

target_link_libraries(taller_server tomlplusplus::tomlplusplus)
