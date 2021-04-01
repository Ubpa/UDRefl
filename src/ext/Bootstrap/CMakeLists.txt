if(NOT Ubpa_UDRefl_Build_ext_Bootstrap)
  return()
endif()

set(c_options_private "")
if(MSVC)
  list(APPEND c_options_private "/MP")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #
endif()

set(mode "")
if(Ubpa_UDRefl_Build_Shared)
  set(mode SHARED)
else()
  set(mode STATIC)
endif()

Ubpa_AddTarget(
  MODE ${mode}
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl_ext/Bootstrap.h"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  LIB
    Ubpa::UDRefl_core
  C_OPTION_PRIVATE
    ${c_options_private}
  PCH_REUSE_FROM UDRefl_core
)
