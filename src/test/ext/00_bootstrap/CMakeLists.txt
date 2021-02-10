if(NOT Ubpa_UDRefl_Build_ext_Bootstrap)
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::UDRefl_ext_Bootstrap
)
