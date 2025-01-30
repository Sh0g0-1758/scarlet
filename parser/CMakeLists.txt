add_library(scarParser STATIC
    parser.cc
    parser.hh
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
