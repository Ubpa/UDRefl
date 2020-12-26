if(NOT UDReflBuildBigObj)
  message(WARNING "skip a target because the current environment doesn't support building a big obj")
  return()
endif()

Ubpa_AddTarget(
  MODE STATIC
  RET_TARGET_NAME tname
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl_pch.h"
  LIB
    Ubpa::UDRefl_core
)

target_precompile_headers(${tname} PRIVATE "${PROJECT_SOURCE_DIR}/include/UDRefl_pch.h")
