set(c_options "")
if(MSVC)
  set(c_options "/bigobj")
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  RET_TARGET_NAME tname
  LIB
    Ubpa::UDRefl_core
  C_OPTION
    ${c_options}
)

target_precompile_headers(${tname} REUSE_FROM UDRefl_core)
