cmake_minimum_required(VERSION 3.14)
project(banking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)
option(COVERAGE "Enable coverage reporting" OFF)

add_library(banking STATIC
    Account.cpp
    Transaction.cpp
)

target_include_directories(banking PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(COVERAGE)
    target_compile_options(banking PRIVATE --coverage -O0 -g)
    target_link_options(banking PRIVATE --coverage)
endif()

if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    
    FetchContent_MakeAvailable(googletest)
    
    add_executable(banking_test
        test_account.cpp
        test_transaction.cpp
    )
    
    target_link_libraries(banking_test PRIVATE banking gtest_main gmock)
    
    if(COVERAGE)
        target_compile_options(banking_test PRIVATE --coverage -O0 -g)
        target_link_options(banking_test PRIVATE --coverage)
        target_link_libraries(banking_test PRIVATE gcov)
    endif()
    
    target_include_directories(banking_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    add_test(NAME banking_test COMMAND banking_test)
endif()
