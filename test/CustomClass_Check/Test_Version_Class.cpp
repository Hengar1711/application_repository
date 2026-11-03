#include <map>
#include <numeric>
#include <vector>
#include <limits>
#include <iostream>
#include <chrono>

#include "../../include/intervall_map.hpp"

std::map<std::size_t, int64_t> Diagnostig_Time;
int test_version = 0;

void IntervalMapTest()
{
    // Testmap.assignselection = test_version;
    // for(auto testinfo : Testcase1)
    // {
    //     auto MemSize = Testmap.m_map.size();

    //     auto startcount = std::chrono::steady_clock::now();        

    //     Testmap.assign(testinfo.keybegin, testinfo.keyend, testinfo.input);

    //     auto endcount = std::chrono::steady_clock::now();

    //     if(Diagnostig_Time[MemSize] == 0)
    //     {
    //         Diagnostig_Time[MemSize] = std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count();
    //     }
    //     else
    //     {
    //         Diagnostig_Time[MemSize] = (Diagnostig_Time[MemSize] + std::chrono::duration_cast<std::chrono::nanoseconds>(endcount - startcount).count()) / 2;
    //     }

    //     InterpredSupport(Testmap.m_map, testinfo.erwartung, testinfo.keybegin, testinfo.keyend, testinfo.input);
    // }    
};


int main(int argc, char**argv)
{
    return 0;
};