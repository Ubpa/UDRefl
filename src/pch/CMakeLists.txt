set(c_options "")
if(MSVC)
  set(c_options "/bigobj")
endif()

Ubpa_AddTarget(
  MODE STATIC
  RET_TARGET_NAME tname
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl_pch.h"
  LIB
    Ubpa::UDRefl_core
  C_OPTION
    ${c_options}
)

target_precompile_headers(${tname} PRIVATE "${PROJECT_SOURCE_DIR}/include/UDRefl_pch.h")
