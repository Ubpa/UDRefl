if(NOT Ubpa_UDRefl_Build_BootStrap)
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::UDRefl_boostrap
)
