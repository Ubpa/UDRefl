if(NOT GTest_FOUND)
  return()
endif()

Ubpa_AddTarget(
  TEST
  MODE EXE
  LIB
    Ubpa::UDRefl_core
    GTest::gtest
    GTest::gtest_main
)