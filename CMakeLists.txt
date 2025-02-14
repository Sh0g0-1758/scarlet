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
    -fno-strict-aliasing
    -fwrapv
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
    COMMAND echo "Running lexer test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage lex
    COMMAND echo "Running parser test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage parse
    COMMAND echo "Running semantic analysis test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage validate
    COMMAND echo "Running scar test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage tacky
    COMMAND echo "Running Codegen test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --stage codegen
    COMMAND echo "Running bitwise, compound, increment/decrement and goto test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6 --extra-credit
    COMMAND echo "Running test asm"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 6
)
