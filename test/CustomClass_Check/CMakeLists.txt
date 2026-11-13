
add_executable(Test_ClassMap Test_Version_Class.cpp)

add_test(NAME TestVs1 COMMAND Test_ClassMap 1)

add_test(NAME TestVs2 COMMAND Test_ClassMap 2)

add_test(NAME TestVs3 COMMAND Test_ClassMap 3)

add_test(NAME TestVs4 COMMAND Test_ClassMap 4)

add_executable(Test_ClassMapSPS Test_Version_Zyklus.cpp)

add_test(NAME TestVsSPS COMMAND Test_ClassMapSPS)