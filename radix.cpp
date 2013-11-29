#include <vector>
#include <array>
#include <random>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <iterator>
#include "../cxx-prettyprint/prettyprint.hpp"
#include <assert.h>
#include <thread>
#include <future>

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
//with iterators, to fit stdlib pattern
template <typename Iter>
void radix_sort(Iter begin, Iter end, const uint & max_digits=8) {
    //default of 8 is the max number of hex digits in an unsigned int 
    std::array<std::vector<uint>,16> buckets;
    for (auto & v : buckets) v.reserve(std::distance(begin,end)/16 * 2); 
    //reserve double the avg bucket size 
    //go through each digit
    for (int i = 0; i < max_digits; ++i) { 
        //put into buckets based on lsd
        for (auto temp = begin; temp != end; ++temp) {
            size_t shift = i * 4;
            size_t index = (*temp >> shift) & 0xF; 
            buckets[index].emplace_back(*temp);
        }
        //put back into vector
        auto temp = begin;
        for (auto & v : buckets) {
            for (const auto & num : v ) {
                *temp= num;
                ++temp;
            }
            v.clear();
        }
    }
}

//Super optimized attempt, no copy back to vector
//emphasis on attempt
std::vector<std::vector<uint>>
opt_radix_sort(std::vector<std::vector<uint>> & buckets, uint i);

template <typename Iter>
void opt_radix_sort(Iter begin, Iter end) {
    //default of 8 is the max number of hex digits in an unsigned int 
    std::vector<std::vector<uint>> buckets(16);
    for (auto & v : buckets) v.reserve(std::distance(begin,end)/16 * 2); 

    //create the first bucket
    for (auto temp = begin; temp != end; ++temp) {
        size_t shift = 0;
        size_t index = (*temp >> shift) & 0xF; 
        buckets[index].emplace_back(*temp);
    }

    for (int i = 1; i < 8; ++i) {
        buckets = opt_radix_sort(buckets,i);
    }

    //put it back into the vector
    for (auto & v : buckets) {
        for (const auto & num : v ) {
            *begin= num;
            ++begin;
        }
    }
}

std::vector<std::vector<uint>>
opt_radix_sort(std::vector<std::vector<uint>> & buckets, uint i) {
    std::vector<std::vector<uint>> next_buckets(16);
    //create next bucket
    for (auto & v : next_buckets) v.reserve(buckets.front().size() * 2); 
    for (const auto & v : buckets) {
        for (const auto & num : v) {
            size_t shift = i * 4;
            size_t index = (num >> shift) & 0xF;
            next_buckets[index].emplace_back(num);
        }
    }
    return next_buckets;
}
//



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

template <typename Iter>
void msd_radix(Iter begin, Iter end, int i = 31) {
    auto upper = end - 1;
    auto lower = begin;
    while (upper != lower) {
        if ( ((1<<i) & *lower) == 0 ) {
            ++lower;
        }
        else {
            std::iter_swap(lower,upper);
            --upper;
        }
    }
    if (i == 0) return;
    if (((1<<i)&*lower)==0) ++lower;
    --i;
    if (begin != lower) {
        msd_radix(begin,lower,i);
    }
    if (lower != end) {
        msd_radix(lower,end,i);
    }
}

//inplace sort
template <typename Iter>
void msd16_radix(Iter begin, Iter end, int i = 7) {
    std::array<uint,16> digit_count;digit_count.fill(0);
    //count the occurence of each digit to place the iterators
    for(auto iter = begin; iter != end; ++iter) {
        ++(digit_count[(*iter >> (i * 4)) & 0xF]);
    }
    std::array<Iter,16> begin_iterators;
    std::array<Iter,16> end_iterators;
    {
        size_t index = 0;
        //create the begin and end iterators
        for (size_t j = 0; j < 16; ++j) {
            begin_iterators[j] = begin + index;
            index += digit_count[j];
            end_iterators[j] = begin + index;
        }
    }
    {
        auto temp_iters = begin_iterators;
        size_t current_bin = 0;
        while (current_bin != 15) {
            if (temp_iters[current_bin] == end_iterators[current_bin]) {
                ++current_bin;
                continue;
            }
            size_t index = (*(temp_iters[current_bin]) >> (i*4)) & 0xF;
            std::iter_swap(temp_iters[current_bin],temp_iters[index]);
            ++(temp_iters[index]);
        }
    }
    if (i == 0) return;
    --i;
    std::vector<std::thread> threads;
    for (size_t j = 0; j < 16; ++j) {
        if(begin_iterators[j] != end_iterators[j]) {
            if (i > 4)
                threads.emplace_back(msd16_radix<Iter>,
                        begin_iterators[j],
                        end_iterators[j],
                        i);
            else msd16_radix(begin_iterators[j],end_iterators[j],i);
        }
    }
    for (auto & t : threads) t.join();
}


int main() {

    //create arrays
    std::vector<uint> v1;
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<uint> dist(1,0xFFFFFFFF);
    for (int i = 0; i < 5000000; ++i) {
        v1.push_back(dist(eng));
    }
    auto v2 = v1;
    auto v3 = v1;
    auto v4 = v1;
    //

    //test with radix_sort
    auto start = std::chrono::system_clock::now();
    //opt_radix_sort(v1.begin(),v1.end());
    radix_sort(v1.begin(),v1.end());
    auto end = std::chrono::system_clock::now();
    auto radix_time = (end-start).count();
    //

    //test with parallel msd inplace radix sort
    start = std::chrono::system_clock::now();
    msd16_radix(v4.begin(),v4.end());
    end = std::chrono::system_clock::now();
    auto parallel_radix_time = (end-start).count();
    //

    //test with sort
    start = std::chrono::system_clock::now();
    std::sort(v2.begin(),v2.end());
    end = std::chrono::system_clock::now();
    auto std_sort_time = (end-start).count();
    //
    
    //test with stable_sort
    start = std::chrono::system_clock::now();
    std::stable_sort(v3.begin(),v3.end());
    end = std::chrono::system_clock::now();
    auto std_stable_sort_time = (end-start).count();
    //
    
    //check successful sort
    for (size_t i = 0; i < v1.size(); ++i) {
        assert(v2[i]==v1[i]);
        assert(v2[i]==v4[i]);
    }

    //prevent sorts from being optimized out
    std::cout << "Ignore these numbers they are printed to prevent optimizations\n";
    std::cout << v1.back() << std::endl;
    std::cout << v2.back() << std::endl;
    std::cout << v3.back() << std::endl;
    std::cout << v4.back() << std::endl;

    std::cout << "radix took: " << radix_time << std::endl;
    std::cout << "parallelized msd inplace radix sort took: " 
        << parallel_radix_time << std::endl;
    std::cout << "std::sort took: " << std_sort_time << std::endl;
    std::cout << "std::stable_sort took: " << std_stable_sort_time << std::endl;
    return 0;
}
