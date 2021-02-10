cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDRefl VERSION 0.8.2)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

Ubpa_AddDep(UTemplate 0.7.0)

include(cmake/Platform.cmake)

option(Ubpa_UDRefl_Build_ext_Bootstrap "build ext Bootstrap" OFF)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
