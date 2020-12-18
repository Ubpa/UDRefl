Ubpa_AddTarget(
  MODE INTERFACE
  RET_TARGET_NAME tname
  LIB
    Ubpa::UDRefl_core
)

target_precompile_headers(${tname} INTERFACE
  $<BUILD_INTERFACE:"${PROJECT_SOURCE_DIR}/include/UDRefl/UDRefl.h">
  $<INSTALL_INTERFACE:${package_name}/include/UDRefl/UDRefl.h>
)
