add_library(scar STATIC
    scar.cc
    scar.hh
)

set_basic_compile_options(scar)

add_dependencies(
    scar
    scarUnop
    scarBinop
  )

target_include_directories(scar PUBLIC ${SCARLET_SOURCE_DIR})
target_link_libraries(scar PUBLIC scarUnop scarBinop)
