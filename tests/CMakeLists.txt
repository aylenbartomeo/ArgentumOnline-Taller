add_executable(taller_tests 
    TestProtocol.cpp 
    TestServerArchitecture.cpp
    TestMap.cpp
    TestWorld.cpp
    TestGameLoop.cpp
    TestPlayer.cpp
    TestBank.cpp
    ../server/src/ConnectionMonitor.cpp
    ../server/src/GameLoop.cpp
    ../server/src/Map.cpp
    ../server/src/World.cpp
    ../server/src/model/entities/components/InventoryComponent.cpp
    ../server/src/model/entities/components/EquipmentComponent.cpp
    ../server/src/model/entities/components/BankComponent.cpp
    ../server/src/model/FormulaEngine.cpp
    ../server/src/model/items/Armor.cpp
    ../server/src/model/items/Weapon.cpp
    ../server/src/model/items/BodyArmor.cpp
    ../server/src/model/items/Helmet.cpp
    ../server/src/model/items/Shield.cpp
)

target_include_directories(taller_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/server/include
    ${CMAKE_SOURCE_DIR}/server/src
    ${CMAKE_SOURCE_DIR}/server/src/model
    ${CMAKE_SOURCE_DIR}/server/src/model/entities
    ${CMAKE_SOURCE_DIR}/server/src/model/items
    .
)

target_link_libraries(taller_tests PRIVATE
    GTest::gtest_main
    lib_common
    tomlplusplus::tomlplusplus
)