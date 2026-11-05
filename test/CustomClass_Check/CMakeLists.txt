
add_executable(Test_ClassMap Test_Version_Class.cpp)

add_test(NAME TestVs1 COMMAND Test_ClassMap 1)

add_test(NAME TestVs2 COMMAND Test_ClassMap 2)

add_test(NAME TestVs3 COMMAND Test_ClassMap 3)