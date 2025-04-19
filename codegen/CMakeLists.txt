add_library(scarCodegen STATIC
    codegen.cc
    codegen.hh
    common.hh
    cfg/cfg.hh

    scargen/scar.cc
    scargen/scar_statement.cc
    scargen/scar_factor.cc
    scargen/scar_expression.cc
    scargen/scar_declaration.cc
    scargen/scar_block.cc
    scargen/pretty_print.cc

    opt/opt.cc
    opt/constant_folding.cc
    opt/unreachable_code_elimination.cc
    opt/copy_propagation.cc

    scasmgen/scasm_generation.cc
    scasmgen/binop_scasm_generation.cc
    scasmgen/unop_scasm_generation.cc
    scasmgen/funcall_scasm_generation.cc
    scasmgen/type_cast_scasm_generation.cc
    scasmgen/fix_pseudo_registers.cc
    scasmgen/fix_instructions.cc
)

set_basic_compile_options(scarCodegen)

add_dependencies(
    scarCodegen
    scar
    scasm
    scarBinop
    scast
  )

target_include_directories(scarCodegen PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scarCodegen PUBLIC scar scasm scarBinop scast)
