cmake_minimum_required(VERSION 3.16)

project(osc-protocol-tests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip # googletest v1.16.x
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
    tests.cpp
)

# target_include_directories(tests PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/../include
# )

target_link_libraries(tests
    GTest::gtest_main
    osc-protocol
)

include(GoogleTest)
gtest_discover_tests(tests)
