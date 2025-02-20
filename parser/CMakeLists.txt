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

    semantic_analysis/analyze_block.cc
    semantic_analysis/analyze_declaration.cc
    semantic_analysis/analyze_expression.cc
    semantic_analysis/analyze_statement.cc

    pretty_print/pretty_print_block.cc
    pretty_print/pretty_print_declaration.cc
    pretty_print/pretty_print_expression.cc
    pretty_print/pretty_print_statement.cc
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
