add_library(scarLexer STATIC
    lexer.cc
    lexer.hh
)

set_basic_compile_options(scarLexer)

add_dependencies(
    scarLexer
    scarRegex
    scarToken
  )

target_include_directories(scarLexer PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scarLexer PUBLIC scarRegex scarToken)
