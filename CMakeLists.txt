cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDRefl VERSION 0.5.4)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_AddDep(UContainer 0.0.9)
Ubpa_AddDep(UTemplate 0.5.4)

include(cmake/Platform.cmake)

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)
