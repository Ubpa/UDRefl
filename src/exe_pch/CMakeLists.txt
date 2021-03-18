if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # work around
  # Clang: __PIC__ level differs in PCH file vs. current file
  # ref: https://izziswift.com/static-analyser-issues-with-command-line-tools/
  Ubpa_AddTarget(
    TEST
    MODE EXE
    PCH
      "${PROJECT_SOURCE_DIR}/include/UDRefl/UDRefl.hpp"
    LIB
      Ubpa::UDRefl_core
  )
else()
  Ubpa_AddTarget(
    TEST
    MODE EXE
    PCH_REUSE_FROM UDRefl_core
    LIB
      Ubpa::UDRefl_core
  )
endif()
