#include <vector>
#include <array>
#include <random>
#include <iostream>
#include <chrono>
#include <algorithm>
//#include "cxx-prettyprint/prettyprint.hpp"
#include <assert.h>

using uint = unsigned int; 

void radix_sort(std::vector<uint> & list, const uint & max_digits=8) {
    //default of 8 is the max number of hex digits in an unsigned int 
    std::array<std::vector<uint>,16> buckets;
    for (auto & v : buckets) v.reserve(list.size()/16 * 2); 
    //reserve double the avg bucket size 
    //go through each digit
    for (int i = 0; i < max_digits; ++i) { 
        //put into buckets based on lsd
        for (const auto & num : list) {
            size_t shift = i * 4;
            size_t index = (num >> shift) & 0xF; 
            buckets[index].emplace_back(num);
        }
        //put back into vector
        uint index = 0;
        for (auto & v : buckets) {
            for (const auto & num : v ) {
                list[index] = num;
                ++index;
            }
            v.clear();
        }
    }
}
//could defintely optimize more by not copying back to the original vector 
//every time 

//not nearly as fast as hex
void radix_sort_binary(std::vector<uint> & list) {
    uint pows = 1;
    for (int i = 0; i < 32; ++i) {
        std::vector<uint> zeroes;
        std::vector<uint> ones;
        for (const auto & num : list) {
            if ( ((1<<i) & num) == 0 ) { //bit is 0 goes to right
                zeroes.emplace_back(num);
            }
            else {
                ones.emplace_back(num);
            }
        }
        size_t index = 0;
        for (const auto & num : zeroes) {
            list[index] = num;
            ++index;
        }
        for (const auto & num : ones) {
            list[index] = num;
            ++index;
        }
        zeroes.clear();
        ones.clear();
    }
}

int main() {
    //create arrays
    std::vector<uint> v1;
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<uint> dist(1,0xFFFF);
    for (int i = 0; i < 5000000; ++i) {
        v1.push_back(dist(eng));
    }
    auto v2 = v1;
    auto v3 = v1;
    //

    //test with radix_sort
    auto start = std::chrono::system_clock::now();
    radix_sort(v1);
    auto end = std::chrono::system_clock::now();
    auto radix_time = (end-start).count();
    //
    
    //test with sort
    start = std::chrono::system_clock::now();
    std::sort(v2.begin(),v2.end());
    end = std::chrono::system_clock::now();
    auto std_sort_time = (end-start).count();
    //
    
    //test with stable_sort
    start = std::chrono::system_clock::now();
    std::sort(v3.begin(),v3.end());
    end = std::chrono::system_clock::now();
    auto std_stable_sort_time = (end-start).count();
    //
    
    //check successful sort
    for (size_t i = 0; i < v1.size(); ++i) {
        assert(v2[i]==v1[i]);
    }

    //prevent sorts from being optimized out
    std::cout << v1.back() << std::endl;
    std::cout << v2.back() << std::endl;
    std::cout << v3.back() << std::endl;

    std::cout << "radix took: " << radix_time << std::endl;
    std::cout << "std::sort took: " << std_sort_time << std::endl;
    std::cout << "std::stable_sort took: " << std_stable_sort_time << std::endl;
    return 0;
}
