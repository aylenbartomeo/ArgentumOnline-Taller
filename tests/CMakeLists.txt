add_executable(taller_tests 
    TestProtocol.cpp 
    TestServerArchitecture.cpp
    TestMap.cpp
    TestWorld.cpp
    ../server/src/ConnectionMonitor.cpp
    ../server/src/Map.cpp
    ../server/src/World.cpp

    # === Núcleo del Modelo ===
    ${CMAKE_SOURCE_DIR}/server/src/model/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/FormulaEngine.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/inventory/inventory.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/Equipment.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/ItemRegistry.cpp
    ${CMAKE_SOURCE_DIR}/server/src/config/ItemConfigLoader.cpp
    # === Ítems y Armas (Esenciales para que cierre el Linker) ===
    ${CMAKE_SOURCE_DIR}/server/src/model/items/Weapon.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/WeaponFactory.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/Armor.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/ArmorFactory.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/BodyArmor.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/Helmet.cpp
    ${CMAKE_SOURCE_DIR}/server/src/model/items/Shield.cpp
)

target_include_directories(taller_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/server/include
    ${CMAKE_SOURCE_DIR}/server/src
    ${CMAKE_SOURCE_DIR}/server/src/model
    ${CMAKE_SOURCE_DIR}/server/src/model/entities
    ${CMAKE_SOURCE_DIR}/server/src/model/items
    ${CMAKE_SOURCE_DIR}/server/src/model/inventory
    .
)

target_link_libraries(taller_tests PRIVATE
    GTest::gtest_main
    lib_common
    tomlplusplus::tomlplusplus
)