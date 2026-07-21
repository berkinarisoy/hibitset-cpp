# ```hibitset-cpp```
![CI](https://github.com/berkinarisoy/hibitset-cpp/actions/workflows/ci.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

A header-only C++20 hierarchical bitset library with fast successor/predecessor queries.

`hibitset::bitset<U>` stores a set of indices in $[0, U)$ as a flat, multi-layer bitmap. The design is a C++ take on the structure behind Rust's [`hibitset`](https://github.com/amethyst/hibitset), generalized to arbitrary universe sizes with the layer count derived at compile time.

## Quick demo
```cpp
#include <hibitset/hibitset.hpp>

hibitset::bitset<262144> bitset;        // universe [0, 262144), 3 layers

bitset.set(50'000);
bitset.set(50'250);

bitset.test(50'000);                    // true
bitset.next_set(50'001);                // 50250  (first set bit >= 50001)
bitset.prev_set(50'249);                // 50000  (last set bit  <= 50249)
bitset.next_set(60'000);                // hibitset::bitset<262144>::npos (none found)

bitset.count();                         // 2
for (auto p : bitset) {}                // iterates 50000 and 50250 in order  

bitset.clear(50'000);
```

Since all operations are ```constexpr```, the whole structure can be built and queried at compile time.

## Why ```hibitset```?
The traditional bitsets find the successor/predecessor of a certain index by performing a linear scan, which is computationally unfeasible for applications that have large universe sizes and require fast iteration. ```hibitset``` solves this problem by creating summary layers that allows the set to skip chunks that doesn't contain any set bits. In the worst case scenario, finding the next/prev set bit will require an ascend and descend through $O(\log_{64} U)$ layers, along with a linear scan on the highest layer of size $O(64)$.

In terms of memory, ```hibitset``` uses a fixed $U/8$ bytes for the base layer plus a small overhead (approximately $1.6\%$) for the summary layers, regardless of how many bits are actually set. 

## Benchmarks
Benchmark results show that ```hibitset``` provides a significant speedup over ```std::set``` and ```boost::dynamic_bitset``` (```dynset```) in successor/predecessor queries, while staying in a competitive range of memory usage. While ```hibitset``` supports algebraic operations, algebra-heavy applications may prefer ```dynset``` for better performance in set algebra operations.

The results can be reproduced by running:
```sh
cmake -B build-bench -S . -DCMAKE_BUILD_TYPE=Release -DHIBITSET_BUILD_BENCHMARKS=ON
cmake --build build-bench -j
./build-bench/benchmarks/hibitset_bench
```

### Set bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 0.57 | 🥉 43.5 | 🥈 1.08 |
| 2^12 (4,096) | 50% | 🥇 0.57 | 🥉 43.0 | 🥈 1.08 |
| 2^12 (4,096) | 95% | 🥇 0.57 | 🥉 42.9 | 🥈 1.08 |
| 2^18 (262,144) | 5% | 🥇 0.60 | 🥉 89.9 | 🥈 1.12 |
| 2^18 (262,144) | 50% | 🥇 0.62 | 🥉 104.8 | 🥈 1.08 |
| 2^18 (262,144) | 95% | 🥇 0.62 | 🥉 126.3 | 🥈 1.09 |
| 2^24 (16,777,216) | 5% | 🥇 0.83 | 🥉 267.4 | 🥈 1.08 |
| 2^24 (16,777,216) | 50% | 🥇 0.89 | 🥉 789.7 | 🥈 1.14 |
| 2^24 (16,777,216) | 95% | 🥇 0.83 | 🥉 895.2 | 🥈 1.10 |

### Find next set bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.70 | 🥉 15.6 | 🥈 3.42 |
| 2^12 (4,096) | 50% | 🥇 0.94 | 🥉 27.2 | 🥈 4.47 |
| 2^12 (4,096) | 95% | 🥇 0.80 | 🥉 33.4 | 🥈 1.44 |
| 2^18 (262,144) | 5% | 🥇 1.60 | 🥉 44.9 | 🥈 3.98 |
| 2^18 (262,144) | 50% | 🥇 0.94 | 🥉 69.6 | 🥈 4.55 |
| 2^18 (262,144) | 95% | 🥇 0.84 | 🥉 79.4 | 🥈 1.44 |
| 2^24 (16,777,216) | 5% | 🥇 4.02 | 🥉 138.5 | 🥈 4.55 |
| 2^24 (16,777,216) | 50% | 🥇 1.51 | 🥉 343.0 | 🥈 5.24 |
| 2^24 (16,777,216) | 95% | 🥇 1.34 | 🥉 444.0 | 🥈 1.70 |

### Find previous set bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.50 | 🥉 28.8 | 🥈 14.9 |
| 2^12 (4,096) | 50% | 🥇 0.94 | 🥉 41.8 | 🥈 6.99 |
| 2^12 (4,096) | 95% | 🥇 0.89 | 🥉 50.9 | 🥈 1.17 |
| 2^18 (262,144) | 5% | 🥇 3.18 | 🥉 67.7 | 🥈 15.4 |
| 2^18 (262,144) | 50% | 🥇 1.46 | 🥉 92.7 | 🥈 6.91 |
| 2^18 (262,144) | 95% | 🥈 1.38 | 🥉 105.2 | 🥇 1.19 |
| 2^24 (16,777,216) | 5% | 🥇 3.93 | 🥉 193.0 | 🥈 18.5 |
| 2^24 (16,777,216) | 50% | 🥇 1.62 | 🥉 473.2 | 🥈 7.73 |
| 2^24 (16,777,216) | 95% | 🥇 1.34 | 🥉 511.4 | 🥈 1.51 |

### Clear bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 0.56 | 🥇 0.54 | 🥉 1.08 |
| 2^12 (4,096) | 50% | 🥈 0.56 | 🥇 0.54 | 🥉 1.07 |
| 2^12 (4,096) | 95% | 🥈 0.57 | 🥇 0.55 | 🥉 1.07 |
| 2^18 (262,144) | 5% | 🥇 0.60 | 🥉 48.2 | 🥈 1.07 |
| 2^18 (262,144) | 50% | 🥇 0.60 | 🥉 79.6 | 🥈 1.08 |
| 2^18 (262,144) | 95% | 🥇 0.61 | 🥉 87.6 | 🥈 1.08 |
| 2^24 (16,777,216) | 5% | 🥇 0.84 | 🥉 195.1 | 🥈 1.07 |
| 2^24 (16,777,216) | 50% | 🥇 0.84 | 🥉 741.0 | 🥈 1.07 |
| 2^24 (16,777,216) | 95% | 🥇 0.82 | 🥉 1,451 | 🥈 1.25 |

### Iterate over set bits time per element (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.82 | 🥈 3.15 | 🥉 3.89 |
| 2^12 (4,096) | 50% | 🥇 0.76 | 🥈 3.30 | 🥉 3.97 |
| 2^12 (4,096) | 95% | 🥇 0.75 | 🥉 6.81 | 🥈 3.87 |
| 2^18 (262,144) | 5% | 🥇 2.41 | 🥉 10.5 | 🥈 4.02 |
| 2^18 (262,144) | 50% | 🥇 0.95 | 🥉 13.9 | 🥈 3.98 |
| 2^18 (262,144) | 95% | 🥇 0.75 | 🥉 25.8 | 🥈 3.89 |
| 2^24 (16,777,216) | 5% | 🥇 3.64 | 🥉 114.4 | 🥈 6.05 |
| 2^24 (16,777,216) | 50% | 🥇 1.23 | 🥉 115.3 | 🥈 3.72 |
| 2^24 (16,777,216) | 95% | 🥇 1.40 | 🥉 116.4 | 🥈 3.96 |

### AND time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 58.5 | 🥉 3,908 | 🥇 8.11 |
| 2^12 (4,096) | 50% | 🥈 73.9 | 🥉 105,985 | 🥇 8.43 |
| 2^12 (4,096) | 95% | 🥈 53.0 | 🥉 450,162 | 🥇 8.42 |
| 2^18 (262,144) | 5% | 🥈 7,793 | 🥉 603,685 | 🥇 514.9 |
| 2^18 (262,144) | 50% | 🥈 10,307 | 🥉 21,436,333 | 🥇 510.1 |
| 2^18 (262,144) | 95% | 🥈 10,286 | 🥉 65,243,000 | 🥇 494.3 |
| 2^24 (16,777,216) | 5% | 🥈 455,309 | 🥉 152,724,000 | 🥇 53,548 |
| 2^24 (16,777,216) | 50% | 🥈 609,231 | 🥉 2,163,954,000 | 🥇 52,856 |
| 2^24 (16,777,216) | 95% | 🥈 914,573 | 🥉 4,965,341,000 | 🥇 61,855 |

### OR time per operation (ns)
```hibitset``` is better at OR than AND and XOR because the resulting summary layers can be trivially computed from the summary layers of the operands, while AND and XOR require a rebuild of the summary layers from the base layer.

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 16.6 | 🥉 21,425 | 🥇 11.2 |
| 2^12 (4,096) | 50% | 🥈 16.9 | 🥉 215,114 | 🥇 11.1 |
| 2^12 (4,096) | 95% | 🥈 17.7 | 🥉 314,589 | 🥇 11.5 |
| 2^18 (262,144) | 5% | 🥈 1,287 | 🥉 2,309,032 | 🥇 727.1 |
| 2^18 (262,144) | 50% | 🥈 1,210 | 🥉 30,040,000 | 🥇 664.7 |
| 2^18 (262,144) | 95% | 🥈 1,361 | 🥉 54,064,000 | 🥇 675.2 |
| 2^24 (16,777,216) | 5% | 🥈 96,490 | 🥉 317,026,000 | 🥇 52,928 |
| 2^24 (16,777,216) | 50% | 🥈 120,048 | 🥉 3,264,369,000 | 🥇 53,268 |
| 2^24 (16,777,216) | 95% | 🥈 90,832 | 🥉 6,464,307,000 | 🥇 53,183 |

### XOR time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 50.4 | 🥉 20,877 | 🥇 8.08 |
| 2^12 (4,096) | 50% | 🥈 49.4 | 🥉 137,560 | 🥇 8.28 |
| 2^12 (4,096) | 95% | 🥈 50.4 | 🥉 44,167 | 🥇 8.54 |
| 2^18 (262,144) | 5% | 🥈 11,107 | 🥉 2,285,156 | 🥇 516.6 |
| 2^18 (262,144) | 50% | 🥈 10,783 | 🥉 23,142,667 | 🥇 521.6 |
| 2^18 (262,144) | 95% | 🥈 10,553 | 🥉 21,292,333 | 🥇 513.8 |
| 2^24 (16,777,216) | 5% | 🥈 685,340 | 🥉 329,951,000 | 🥇 54,458 |
| 2^24 (16,777,216) | 50% | 🥈 673,337 | 🥉 2,476,673,000 | 🥇 50,362 |
| 2^24 (16,777,216) | 95% | 🥈 638,207 | 🥉 2,333,249,000 | 🥇 71,858 |

### Total initialization time (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 188.1 | 🥉 7,552 | 🥇 124.8 |
| 2^12 (4,096) | 50% | 🥈 1,217 | 🥉 101,205 | 🥇 907.6 |
| 2^12 (4,096) | 95% | 🥈 2,293 | 🥉 233,352 | 🥇 1,677 |
| 2^18 (262,144) | 5% | 🥈 14,583 | 🥉 1,311,400 | 🥇 6,067 |
| 2^18 (262,144) | 50% | 🥈 79,776 | 🥉 20,280,333 | 🥇 53,628 |
| 2^18 (262,144) | 95% | 🥈 157,660 | 🥉 47,186,000 | 🥇 100,315 |
| 2^24 (16,777,216) | 5% | 🥈 2,690,556 | 🥉 383,574,000 | 🥇 746,989 |
| 2^24 (16,777,216) | 50% | 🥈 10,348,833 | 🥉 8,799,128,000 | 🥇 8,256,667 |
| 2^24 (16,777,216) | 95% | 🥈 18,000,000 | 🥉 19,840,866,000 | 🥇 15,695,000 |

### Memory usage
```hibitset``` gains major query speedups while staying in a competitive range of ```dynset``` in memory usage.

| Universe size | Fill density | hibitset (bytes) | stdset (bytes) | dynset (bytes) |
|---|---|---|---|---|
| 2^12 (4,096) | 0% | 🥈 528 | 🥇 24 | 🥉 544 |
| 2^12 (4,096) | 0.1% | 🥈 528 | 🥇 184 | 🥉 544 |
| 2^12 (4,096) | 1% | 🥇 528 | 🥉 1,624 | 🥈 544 |
| 2^12 (4,096) | 5% | 🥇 528 | 🥉 8,184 | 🥈 544 |
| 2^12 (4,096) | 10% | 🥇 528 | 🥉 16,384 | 🥈 544 |
| 2^12 (4,096) | 25% | 🥇 528 | 🥉 40,984 | 🥈 544 |
| 2^12 (4,096) | 50% | 🥇 528 | 🥉 81,944 | 🥈 544 |
| 2^12 (4,096) | 75% | 🥇 528 | 🥉 122,904 | 🥈 544 |
| 2^12 (4,096) | 100% | 🥇 528 | 🥉 163,864 | 🥈 544 |
| 2^18 (262,144) | 0% | 🥉 33,296 | 🥇 24 | 🥈 32,800 |
| 2^18 (262,144) | 0.1% | 🥉 33,296 | 🥇 10,504 | 🥈 32,800 |
| 2^18 (262,144) | 1% | 🥈 33,296 | 🥉 104,864 | 🥇 32,800 |
| 2^18 (262,144) | 5% | 🥈 33,296 | 🥉 524,304 | 🥇 32,800 |
| 2^18 (262,144) | 10% | 🥈 33,296 | 🥉 1,048,584 | 🥇 32,800 |
| 2^18 (262,144) | 25% | 🥈 33,296 | 🥉 2,621,464 | 🥇 32,800 |
| 2^18 (262,144) | 50% | 🥈 33,296 | 🥉 5,242,904 | 🥇 32,800 |
| 2^18 (262,144) | 75% | 🥈 33,296 | 🥉 7,864,344 | 🥇 32,800 |
| 2^18 (262,144) | 100% | 🥈 33,296 | 🥉 10,485,784 | 🥇 32,800 |
| 2^24 (16,777,216) | 0% | 🥉 2,130,448 | 🥇 24 | 🥈 2,097,184 |
| 2^24 (16,777,216) | 0.1% | 🥉 2,130,448 | 🥇 671,104 | 🥈 2,097,184 |
| 2^24 (16,777,216) | 1% | 🥈 2,130,448 | 🥉 6,710,904 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 5% | 🥈 2,130,448 | 🥉 33,554,424 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 10% | 🥈 2,130,448 | 🥉 67,108,864 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 25% | 🥈 2,130,448 | 🥉 167,772,184 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 50% | 🥈 2,130,448 | 🥉 335,544,344 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 75% | 🥈 2,130,448 | 🥉 503,316,504 | 🥇 2,097,184 |
| 2^24 (16,777,216) | 100% | 🥈 2,130,448 | 🥉 671,088,664 | 🥇 2,097,184 |

## Installation

### Install via CMake FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(hibitset
    GIT_REPOSITORY https://github.com/berkinarisoy/hibitset-cpp.git
    GIT_TAG v0.1.0)
FetchContent_MakeAvailable(hibitset)

target_link_libraries(your_target PRIVATE hibitset::hibitset)
```

### Install manually
You can vendor the header file `include/hibitset/hibitset.hpp` into your project and include it in your source files. No additional dependencies are required.

### Requirements
- C++20
- CMake 3.23 (or higher)

## API overview

### Member functions
| | |
|-|-|
| ```test``` | accesses specific bit |
| ```all```, ```any```, ```empty``` | checks if all/any/none of the bits are set |
| ```capacity``` | returns the universe size |
| ```count``` | returns the number of set bits |
| ```first```, ```last``` | returns the first/last set bit |
| ```reset``` | clears all bits |
| ```set``` | sets a specific bit to ```true``` |
| ```clear``` | sets a specific bit to ```false``` |
| ```next_set``` | returns the next set bit at or after a given index |
| ```prev_set``` | returns the previous set bit at or before a given index |
| ```operator==```, ```operator!=``` | compares two bitsets for equality |
| ```operator\|=```, ```operator&=```, ```operator^=``` | performs bitwise OR, AND, XOR in-place |
| ```operator\|```, ```operator&```, ```operator^``` | performs bitwise OR, AND, XOR and returns a new bitset |
| ```npos``` | returns a sentinel value for "not found" |
| ```begin```, ```end``` | returns iterators to the first and one-past-the-last set bits |
| ```cbegin```, ```cend``` | returns const iterators to the first and one-past-the-last set bits |

## Behind the scenes
```
U = 4096, 2 layers, set bits = {1, 65, 4095}
--------------------------------------------------------------
Layer 1: 1 -----  1 -----  0 -----  ...  0 -----  1 -----
Layer 0: 0...010  0...010  0...000  ...  0...000  10...00
         chunk 0  chunk 1  chunk 2  ...  chunk 62 chunk 63
--------------------------------------------------------------
```
In the example above, the base layer (layer 0) is a flat bitmap of $64$ chunks and the summary layer (layer 1) consists of a single chunk, where each chunk is a ```uint64_t``` that represents 64 bits. The higher layers are summary layers that indicate whether any bits are set in the corresponding chunks of the lower layer. When we call ```next_set(2)```, the algorithm first checks whether the chunk containing index 2 (chunk 0) has any set bits at or after index 2 by looking at layer 0. Since only the first bit of chunk 0 is set, it ascends to layer 1 and check whether any bits are set in the chunks after chunk 0. It finds that chunk 1 has a set bit, so it descends back to layer 0 and finds the next set bit at index 65. Same process happens for ```next_set(66)```, where it finds the next set bit at index 4095. Instead of scanning through all the bits in layer 0, the algorithm can skip over the chunks that don't have any set bits by using the summary layers. For large universe sizes, the algorithm can ascend to higher layers and skip over many chunks, making these queries very efficient.