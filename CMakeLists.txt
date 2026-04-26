cmake_minimum_required(VERSION 3.14)
project(banking)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(COVERAGE "Enable coverage reporting" OFF)

if(COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

add_library(banking STATIC
    Account.cpp
    Transaction.cpp
)

target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    FetchContent_Declare(
        trompeloeil
        GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
        GIT_TAG v43
    )
    
    FetchContent_MakeAvailable(googletest trompeloeil)
    
    add_executable(banking_test
        test_account.cpp
        test_transaction.cpp
    )
    
    target_link_libraries(banking_test PRIVATE
        banking
        gtest_main
        trompeloeil
    )
    
    target_include_directories(banking_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${trompeloeil_SOURCE_DIR}/include
    )
    
    add_test(NAME banking_test COMMAND banking_test)
endif()
