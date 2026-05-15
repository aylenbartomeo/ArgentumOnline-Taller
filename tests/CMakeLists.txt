add_executable(taller_tests 
    TestProtocol.cpp 
)

target_link_libraries(taller_tests PRIVATE
    GTest::gtest_main
    lib_common
)