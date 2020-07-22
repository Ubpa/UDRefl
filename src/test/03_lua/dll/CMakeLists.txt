if(NOT Ubpa_UDRefl_Test_lua)
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE SHARED
  RET_TARGET_NAME UDRefl_lua
  LIB
    Ubpa::ULua_core
	Ubpa::UDRefl_core
  DEFINE_PRIVATE
    UBPA_EXPORT_UDREFL_LUA
)

set_target_properties(${UDRefl_lua} PROPERTIES OUTPUT_NAME UDRefl_Lua CLEAN_DIRECT_OUTPUT 1)
