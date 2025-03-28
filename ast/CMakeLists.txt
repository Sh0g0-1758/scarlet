add_library(scast STATIC
    ast.cc
    ast.hh
)

set_basic_compile_options(scast)

add_dependencies(
    scast
    scarUnop
    scarBinop
  )

target_include_directories(scast PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scast PUBLIC scarUnop scarBinop)
