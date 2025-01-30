add_library(scasm STATIC
    scasm.cc
    scasm.hh
)

set_basic_compile_options(scasm)

add_dependencies(
    scasm
    scarUnop
    scarBinop
  )

target_include_directories(scasm PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scasm PUBLIC scarUnop scarBinop)
