cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDRefl VERSION 0.9.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

Ubpa_AddDep(UTemplate 0.7.1)

include(cmake/Platform.cmake)

option(Ubpa_UDRefl_Build_ext_Bootstrap "build ext Bootstrap" OFF)
option(Ubpa_UDRefl_include_all_StdName "switch UBPA_UDREFL_INCLUDE_ALL_STD_NAME" OFF)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
