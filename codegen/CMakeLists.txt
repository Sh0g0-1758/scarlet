add_library(scarCodegen STATIC
    codegen.cc
    codegen.hh
)

set_basic_compile_options(scarCodegen)

add_dependencies(
    scarCodegen
    scarToken
    scar
    scasm
  )

target_include_directories(scarCodegen PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scarCodegen PUBLIC scarToken scar scasm)
