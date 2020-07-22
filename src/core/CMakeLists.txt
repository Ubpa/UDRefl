set(c_options "")
if(MSVC)
  list(APPEND c_options "/wd5030;/wd4251")
endif()

Ubpa_AddTarget(
  MODE SHARED
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  C_OPTION
    ${c_options}
  DEFINE
    UBPA_UDREFL_BUILD_AS_SHARED
  DEFINE_PRIVATE
    UBPA_UDREFL_EXPORT
)
