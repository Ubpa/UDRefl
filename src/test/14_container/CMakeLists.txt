Ubpa_AddTarget(
  TEST
  MODE EXE
  RET_TARGET_NAME tname
  LIB
    Ubpa::UDRefl_core
  C_OPTION
    /bigobj
)

target_precompile_headers(${tname} REUSE_FROM UDRefl_core)