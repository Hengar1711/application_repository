
add_executable(Test_IntervalMap Test_Version_Calc.cpp)

add_test(NAME TestVs1 COMMAND Test_IntervalMap 1)

add_test(NAME TestVs2 COMMAND Test_IntervalMap 2)

add_test(NAME TestVs3 COMMAND Test_IntervalMap 3)