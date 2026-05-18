add_executable(taller_tests 
    TestProtocol.cpp 
    TestServerArchitecture.cpp
    TestMap.cpp
    TestWorld.cpp
    TestGameLoop.cpp
    ../server/src/ConnectionMonitor.cpp
    ../server/src/GameLoop.cpp
    ../server/src/Map.cpp
    ../server/src/World.cpp
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