
add_executable(Test_IntervalMap Test_Version_Calc.cpp)

add_test(NAME TestVs1 COMMAND Test_IntervalMap 1)

add_test(NAME TestVs2 COMMAND Test_IntervalMap 2)

add_test(NAME TestVs3 COMMAND Test_IntervalMap 3)

add_test(NAME TestVs4 COMMAND Test_IntervalMap 4)



add_executable(Test_IntervalMapSPS Test_Version_Zyklus.cpp)

add_test(NAME TestVsSPS COMMAND Test_IntervalMapSPS)