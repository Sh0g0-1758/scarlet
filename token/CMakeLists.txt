add_library(scarToken STATIC
    token.cc
    token.hh
)

target_include_directories(scarToken PUBLIC ${SCARLET_SOURCE_DIR})
set_basic_compile_options(scarToken)
