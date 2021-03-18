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
  PCH_REUSE_FROM UDRefl_exe_pch
)
