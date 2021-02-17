set(c_options "")
if(MSVC)
  list(APPEND c_options "/wd5030;/bigobj")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  list(APPEND c_options "-Wno-attributes")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND c_options "-Wno-attributes")
endif()

set(defines "")
if(MSVC)
  list(APPEND defines "_SILENCE_STDEXT_HASH_LOWER_BOUND_DEPRECATION_WARNING")
  list(APPEND defines "_SILENCE_STDEXT_HASH_UPPER_BOUND_DEPRECATION_WARNING")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  list(APPEND defines "")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND defines "")
endif()

if(Ubpa_UDRefl_include_all_StdName)
  list(APPEND defines "UBPA_UDREFL_INCLUDE_ALL_STD_NAME")
endif()

Ubpa_AddTarget(
  MODE STATIC
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  C_OPTION
    ${c_options}
  LIB
    Ubpa::UTemplate_core
  DEFINE
    ${defines}
  PCH
    "${PROJECT_SOURCE_DIR}/include/UDRefl/UDRefl.h"
)
