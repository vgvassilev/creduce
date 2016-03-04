# Test runner infrastructure for Clang. This configures the Clang test trees
# for use by Lit, and delegates to LLVM's lit test handlers.

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

find_program(LIT_COMMAND llvm-lit PATHS ${LLVM_TOOLS_BINARY_DIR})

if (CMAKE_CFG_INTDIR STREQUAL ".")
  set(LLVM_BUILD_MODE ".")
else ()
  set(LLVM_BUILD_MODE "%(build_mode)s")
endif ()

set(CLANGDELTA_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CLANGDELTA_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(CLANGDELTA_BINARY_DIR ${PROJECT_BINARY_DIR})

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  @ONLY
  )

set(CLANGDELTA_TEST_PARAMS
  clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

#list(APPEND CLANGDELTA_TEST_DEPS
#  llvm-config
#  FileCheck count not
#  llc
#  llvm-bcanalyzer
#  llvm-lto
#  llvm-objdump
#  llvm-profdata
#  llvm-readobj
#  llvm-symbolizer
#  LTO
#  opt
#  )

set(LIT_ARGS ${LIT_ARGS} "-sv")

option(CLANGDELTA_TEST_USE_VG "Run ClangDelta tests under Valgrind" OFF)
if(CLANGDELTA_TEST_USE_VG)
  set(LIT_ARGS ${LIT_ARGS} "--vg")
endif ()


add_lit_testsuite(check-clangdelta "Running the ClangDelta regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  #LIT ${LLVM_LIT}
  PARAMS ${CLANGDELTA_TEST_PARAMS}
  DEPENDS ${CLANGDELTA_TEST_DEPS}
  ARGS ${LIT_ARGS}
  )
set_target_properties(check-clangdelta PROPERTIES FOLDER "ClangDelta tests")

# Add a legacy target spelling: clang-test
add_custom_target(clangdelta-test)
add_dependencies(clangdelta-test check-clangdelta)
set_target_properties(clangdelta-test PROPERTIES FOLDER "ClangDelta tests")
