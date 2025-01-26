add_library(scarCodegen STATIC
    codegen.cc
    codegen.hh
    scasm.hh
)

set_basic_compile_options(scarCodegen)

add_dependencies(
    scarCodegen
    scarToken
  )

target_include_directories(scarCodegen PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scarCodegen PUBLIC scarToken)
