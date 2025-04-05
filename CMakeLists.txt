cmake_minimum_required(VERSION 3.14)
project(Scarlet LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in debug mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in release mode")
else()
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}\nValid build types are: Debug, Release")
endif()

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
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3>
  )
endfunction()

add_subdirectory(token)
add_subdirectory(regex)
add_subdirectory(unary_operations)
add_subdirectory(binary_operations)
add_subdirectory(ast)
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
    -Wall
    -Wextra
    -Werror
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3>
)

add_dependencies(
    scarlet
    scast
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
target_link_libraries(scarlet scast)
target_compile_options(scarlet PRIVATE -O3)

add_custom_target(test-scarlet
    DEPENDS scarlet
    COMMAND echo "Running lexer test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 15 --stage lex
    COMMAND echo "Running parser test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 15 --stage parse
    COMMAND echo "Running semantic analysis test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 14 --stage validate
    COMMAND echo "Running scar test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 13 --stage tacky
    COMMAND echo "Running Codegen test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 13 --stage codegen
    COMMAND echo "Running test asm"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 13
    COMMAND echo "Running bitwise, compound, increment/decrement, goto and switch-case test"
    COMMAND ${CMAKE_SOURCE_DIR}/test_compiler ./scarlet --chapter 13 --extra-credit
)
