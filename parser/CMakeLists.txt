add_library(scarParser STATIC
    parser.hh
    common.hh
    parser.cc
    parse_function.cc
    parse_block.cc
    parse_statement.cc
    parse_declaration.cc
    parse_expression.cc
    parse_binop.cc
    parse_unop.cc
    parse_constants.cc
)

set_basic_compile_options(scarParser)

add_dependencies(
    scarParser
    scarUnop
    scarBinop
    scarToken
  )

target_include_directories(scarParser PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scarParser PUBLIC scarUnop scarToken scarBinop)
