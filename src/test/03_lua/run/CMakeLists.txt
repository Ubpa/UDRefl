if(NOT Ubpa_UDRefl_Test_lua)
  return()
endif()

Ubpa_GetTargetName(dll "${PROJECT_SOURCE_DIR}/src/test/03_lua/dll")

Ubpa_AddTarget(
  TEST
  MODE EXE
  SOURCE
    "${PROJECT_SOURCE_DIR}/bin/lua/test.lua"
  LIB
    Ubpa::ULua_core
    Ubpa::UDRefl_core
    ${dll}
)
