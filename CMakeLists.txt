cmake_minimum_required(VERSION 3.14)
project(Scarlet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Using ${CMAKE_CXX_COMPILER} (version ${CMAKE_CXX_COMPILER_VERSION}) to build Scarlet")

set(SCARLET_SOURCE_DIR ${CMAKE_SOURCE_DIR})
execute_process(
  COMMAND uname -m
  OUTPUT_VARIABLE SYSTEM_ARCHITECTURE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(SYSTEM_ARCHITECTURE STREQUAL "arm64")
    message(STATUS "Detected ARM64 platform")
    message(FATAL_ERROR "Build halted.\nPlease run the following command in your terminal and then build again:\n'arch -x86_64 zsh'")
endif()

function(set_basic_compile_options target)
  target_compile_options(${target} PRIVATE 
    -Wall
    -Wextra
    -Werror
    -fno-rtti
    -fpie
    -O3
  )
endfunction()

add_subdirectory(token)
add_subdirectory(regex)
add_subdirectory(unary_operations)
add_subdirectory(binary_operations)
add_subdirectory(scar)
add_subdirectory(scasm)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(codegen)

set(SOURCES ./driver/driver.cc)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(scarlet ${SOURCES})
target_compile_options(scarlet PRIVATE 
    -O3
    -Wall
    -Wextra
    -Werror
)

add_dependencies(
    scarlet
    scar
    scasm
    scarRegex
    scarLexer
    scarParser
    scarCodegen
  )

target_link_libraries(scarlet ${Boost_LIBRARIES})
target_link_libraries(scarlet scarRegex)
target_link_libraries(scarlet scarLexer)
target_link_libraries(scarlet scarParser)
target_link_libraries(scarlet scarCodegen)
target_link_libraries(scarlet scar)
target_link_libraries(scarlet scasm)
target_compile_options(scarlet PRIVATE -O3)

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
    COMMAND echo "Running parser-3 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3 --stage parse
    COMMAND echo "Running scar-3 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3 --stage tacky
    COMMAND echo "Running Codegen-3 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3 --stage codegen
    COMMAND echo "Running test all-3"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3
    COMMAND echo "Running bitwise test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 3 --bitwise
    COMMAND echo "Running lexer-4 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 4 --stage lex
    COMMAND echo "Running parser-4 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 4 --stage parse
    COMMAND echo "Running scar-4 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 4 --stage tacky
    COMMAND echo "Running lexer-5 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 5 --stage lex
    COMMAND echo "Running lexer-6 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage lex
    COMMAND echo "Running lexer-8 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 8 --stage lex
    COMMAND echo "Running lexer-9 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 9 --stage lex
    COMMAND echo "Running lexer-10 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 10 --stage lex
    COMMAND echo "Running lexer-11 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 11 --stage lex
    COMMAND echo "Running lexer-12 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 12 --stage lex
    COMMAND echo "Running lexer-13 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 13 --stage lex
    COMMAND echo "Running lexer-14 test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 14 --stage lex
)
