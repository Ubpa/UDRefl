cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDRefl VERSION 0.0.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

option(Ubpa_UDRefl_Test_lua "Ubpa UDRefl Test lua" OFF)
if(Ubpa_UDRefl_Test_lua)
  Ubpa_AddDep(ULua 0.1.0)
endif()

Ubpa_AddSubDirsRec(include)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(DIRECTORIES "include")
