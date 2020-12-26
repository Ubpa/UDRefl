if(NOT UDReflBuildBigObj)
  message(WARNING "skip a target because the current environment doesn't support building a big obj")
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  RET_TARGET_NAME tname
  LIB
    Ubpa::UDRefl_core
)

target_precompile_headers(${tname} REUSE_FROM ${UDReflPchTargetName})
