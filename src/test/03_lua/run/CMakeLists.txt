if(NOT Ubpa_UDRefl_Test_lua)
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::ULua_core
)
