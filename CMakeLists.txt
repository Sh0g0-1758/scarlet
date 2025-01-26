cmake_minimum_required(VERSION 3.14)
project(Scarlet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER g++-13)

set(SOURCES ./driver/driver.cc)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(scarlet ${SOURCES})

target_link_libraries(scarlet ${Boost_LIBRARIES})

add_custom_target(test-scarlet
    DEPENDS scarlet
    COMMAND echo "Running lexer-1 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 1 --stage lex
    COMMAND echo "Running parser-1 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 1 --stage parse
    COMMAND echo "Running Codegen-1 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 1 --stage codegen
    COMMAND echo "Running test all-1"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 1
    COMMAND echo "Running lexer-2 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 2 --stage lex
    COMMAND echo "Running parser-2 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 2 --stage parse
    COMMAND echo "Running scar-2 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 2 --stage tacky
    COMMAND echo "Running Codegen-2 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 2 --stage codegen
    COMMAND echo "Running test all-2"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 2
    COMMAND echo "Running lexer-3 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3 --stage lex
)
