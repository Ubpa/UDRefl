cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDRefl VERSION 0.10.4)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

Ubpa_AddDep(UTemplate 0.7.2)
Ubpa_AddDep(USmallFlat 0.2.2)

include(cmake/Platform.cmake)

option(Ubpa_UDRefl_Build_Shared "build shared library" OFF)
option(Ubpa_UDRefl_Build_ext_Bootstrap "build ext Bootstrap" OFF)
option(Ubpa_UDRefl_include_all_StdName "switch UBPA_UDREFL_INCLUDE_ALL_STD_NAME" OFF)

if(Ubpa_BuildTest_UDRefl)
  find_package(GTest QUIET)
  if(GTest_FOUND)
    message(NOTICE "GTest Found")
  else()
    message(NOTICE "GTest not Found, so we ignore some targets depending on GTest")
  endif()
endif()

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
