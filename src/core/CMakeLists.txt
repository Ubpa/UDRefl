set(c_options "")
if(MSVC)
  list(APPEND c_options "/wd5030;/bigobj")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  list(APPEND c_options "-Wno-attributes")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND c_options "-Wno-attributes")
endif()

set(c_options_private "")
if(MSVC)
  list(APPEND c_options_private "/MP")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #
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

set(mode "")
if(Ubpa_UDRefl_Build_Shared)
  set(mode SHARED)
  list(APPEND defines "UBPA_UDREFL_SHARED")
  if(MSVC)
    list(APPEND c_options "/wd4251;/wd4275")
    list(APPEND c_options_private "/wd4651")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    #
  endif()
else()
  set(mode STATIC)
endif()

if(Ubpa_UDRefl_include_all_StdName)
  list(APPEND defines "UBPA_UDREFL_INCLUDE_ALL_STD_NAME")
endif()

Ubpa_AddTarget(
  MODE ${mode}
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDRefl"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  C_OPTION
    ${c_options}
  C_OPTION_PRIVATE
    ${c_options_private}
  LIB
    Ubpa::UTemplate_core
    Ubpa::USmallFlat_core
  DEFINE
    ${defines}
  PCH
    "${PROJECT_SOURCE_DIR}/include/UDRefl/UDRefl.hpp"
)
