set(c_options "")
if(MSVC)
  set(c_options "/bigobj")
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::UDRefl_core
  C_OPTION
    ${c_options}
)
