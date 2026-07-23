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

In terms of memory, ```hibitset``` uses a fixed $U/8$ bytes for the base layer plus a small overhead (approximately 1.6%) for the summary layers, regardless of how many bits are actually set. 

## Benchmarks
Benchmark results show that ```hibitset``` provides a significant speedup over ```std::set``` and ```boost::dynamic_bitset``` (```dynset```) in successor/predecessor queries, while staying in a competitive range of memory usage. While ```hibitset``` supports algebraic operations, algebra-heavy applications may prefer ```dynset``` for better performance in set algebra operations.

The benchmarks are run on Apple M3, Apple Clang 17.0.0, -O3 -DNDEBUG (CMake Release), single-threaded, macOS.

The results can be reproduced by running:
```sh
cmake -B build-bench -S . -DCMAKE_BUILD_TYPE=Release -DHIBITSET_BUILD_BENCHMARKS=ON
cmake --build build-bench -j
./build-bench/benchmarks/hibitset_bench
```

### Find next set bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.46 | 🥉 16.5 | 🥈 3.75 |
| 2^12 (4,096) | 50% | 🥇 0.95 | 🥉 27.8 | 🥈 4.66 |
| 2^12 (4,096) | 95% | 🥇 0.81 | 🥉 34.4 | 🥈 1.46 |
| 2^18 (262,144) | 5% | 🥇 1.82 | 🥉 45.0 | 🥈 4.06 |
| 2^18 (262,144) | 50% | 🥇 0.92 | 🥉 70.9 | 🥈 4.95 |
| 2^18 (262,144) | 95% | 🥇 0.85 | 🥉 83.8 | 🥈 1.44 |
| 2^24 (16,777,216) | 5% | 🥇 3.92 | 🥉 149.2 | 🥈 5.13 |
| 2^24 (16,777,216) | 50% | 🥇 1.53 | 🥉 382.7 | 🥈 5.75 |
| 2^24 (16,777,216) | 95% | 🥇 1.37 | 🥉 445.7 | 🥈 1.73 |

### Find previous set bit time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.51 | 🥉 28.2 | 🥈 14.6 |
| 2^12 (4,096) | 50% | 🥇 0.89 | 🥉 42.3 | 🥈 7.15 |
| 2^12 (4,096) | 95% | 🥇 0.80 | 🥉 50.9 | 🥈 1.15 |
| 2^18 (262,144) | 5% | 🥇 3.18 | 🥉 64.6 | 🥈 15.2 |
| 2^18 (262,144) | 50% | 🥇 1.43 | 🥉 93.3 | 🥈 7.11 |
| 2^18 (262,144) | 95% | 🥈 1.34 | 🥉 108.9 | 🥇 1.16 |
| 2^24 (16,777,216) | 5% | 🥇 3.78 | 🥉 234.0 | 🥈 18.6 |
| 2^24 (16,777,216) | 50% | 🥇 1.56 | 🥉 467.8 | 🥈 8.40 |
| 2^24 (16,777,216) | 95% | 🥇 1.35 | 🥉 543.1 | 🥈 1.52 |

### Iterate over set bits time per element (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥇 1.60 | 🥈 2.49 | 🥉 3.49 |
| 2^12 (4,096) | 50% | 🥇 0.68 | 🥈 2.60 | 🥉 3.59 |
| 2^12 (4,096) | 95% | 🥇 0.67 | 🥉 4.85 | 🥈 3.48 |
| 2^18 (262,144) | 5% | 🥇 2.19 | 🥉 7.92 | 🥈 3.60 |
| 2^18 (262,144) | 50% | 🥇 0.83 | 🥉 12.3 | 🥈 3.56 |
| 2^18 (262,144) | 95% | 🥇 0.66 | 🥉 15.8 | 🥈 3.46 |
| 2^24 (16,777,216) | 5% | 🥇 3.04 | 🥉 60.5 | 🥈 5.13 |
| 2^24 (16,777,216) | 50% | 🥇 0.86 | 🥉 72.3 | 🥈 3.58 |
| 2^24 (16,777,216) | 95% | 🥇 0.70 | 🥉 74.7 | 🥈 3.49 |

### AND time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 51.6 | 🥉 2,457 | 🥇 7.12 |
| 2^12 (4,096) | 50% | 🥈 35.9 | 🥉 66,933 | 🥇 7.01 |
| 2^12 (4,096) | 95% | 🥈 35.9 | 🥉 251,416 | 🥇 7.01 |
| 2^18 (262,144) | 5% | 🥈 6,994 | 🥉 236,871 | 🥇 417.6 |
| 2^18 (262,144) | 50% | 🥈 8,958 | 🥉 9,500,833 | 🥇 417.0 |
| 2^18 (262,144) | 95% | 🥈 8,964 | 🥉 31,829,500 | 🥇 417.1 |
| 2^24 (16,777,216) | 5% | 🥈 387,865 | 🥉 90,221,000 | 🥇 45,770 |
| 2^24 (16,777,216) | 50% | 🥈 576,934 | 🥉 1,569,192,000 | 🥇 46,475 |
| 2^24 (16,777,216) | 95% | 🥈 578,616 | 🥉 4,136,211,000 | 🥇 46,012 |

### OR time per operation (ns)
```hibitset``` is better at OR than AND and XOR because the resulting summary layers can be trivially computed from the summary layers of the operands, while AND and XOR require a rebuild of the summary layers from the base layer.

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 13.3 | 🥉 19,372 | 🥇 7.22 |
| 2^12 (4,096) | 50% | 🥈 13.2 | 🥉 193,502 | 🥇 7.07 |
| 2^12 (4,096) | 95% | 🥈 13.2 | 🥉 274,056 | 🥇 7.38 |
| 2^18 (262,144) | 5% | 🥈 970.9 | 🥉 2,024,000 | 🥇 419.4 |
| 2^18 (262,144) | 50% | 🥈 979.6 | 🥉 22,428,667 | 🥇 420.9 |
| 2^18 (262,144) | 95% | 🥈 980.2 | 🥉 35,328,000 | 🥇 418.2 |
| 2^24 (16,777,216) | 5% | 🥈 82,102 | 🥉 270,159,000 | 🥇 46,146 |
| 2^24 (16,777,216) | 50% | 🥈 82,350 | 🥉 2,623,463,000 | 🥇 55,512 |
| 2^24 (16,777,216) | 95% | 🥈 81,664 | 🥉 4,372,417,000 | 🥇 46,342 |

### XOR time per operation (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 35.3 | 🥉 18,597 | 🥇 7.05 |
| 2^12 (4,096) | 50% | 🥈 36.0 | 🥉 119,992 | 🥇 7.02 |
| 2^12 (4,096) | 95% | 🥈 36.0 | 🥉 36,276 | 🥇 7.01 |
| 2^18 (262,144) | 5% | 🥈 8,803 | 🥉 1,872,135 | 🥇 417.6 |
| 2^18 (262,144) | 50% | 🥈 8,958 | 🥉 14,533,750 | 🥇 416.8 |
| 2^18 (262,144) | 95% | 🥈 8,957 | 🥉 11,637,000 | 🥇 417.3 |
| 2^24 (16,777,216) | 5% | 🥈 561,484 | 🥉 262,352,000 | 🥇 45,802 |
| 2^24 (16,777,216) | 50% | 🥈 585,361 | 🥉 2,088,303,000 | 🥇 45,800 |
| 2^24 (16,777,216) | 95% | 🥈 591,437 | 🥉 1,822,287,000 | 🥇 45,969 |

### Total initialization time (ns)

| Universe size | Fill density | hibitset (ns) | stdset (ns) | dynset (ns) |
|---|---|---|---|---|
| 2^12 (4,096) | 5% | 🥈 171.1 | 🥉 6,489 | 🥇 106.8 |
| 2^12 (4,096) | 50% | 🥈 1,050 | 🥉 89,691 | 🥇 744.1 |
| 2^12 (4,096) | 95% | 🥈 1,907 | 🥉 266,793 | 🥇 1,475 |
| 2^18 (262,144) | 5% | 🥈 11,503 | 🥉 1,134,759 | 🥇 5,147 |
| 2^18 (262,144) | 50% | 🥈 64,450 | 🥉 17,942,500 | 🥇 45,953 |
| 2^18 (262,144) | 95% | 🥈 129,105 | 🥉 38,129,500 | 🥇 87,837 |
| 2^24 (16,777,216) | 5% | 🥈 2,053,727 | 🥉 236,304,000 | 🥇 642,863 |
| 2^24 (16,777,216) | 50% | 🥈 8,438,500 | 🥉 6,531,067,000 | 🥇 6,579,800 |
| 2^24 (16,777,216) | 95% | 🥈 14,887,800 | 🥉 14,431,485,000 | 🥇 14,314,800 |

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
| ```begin```, ```end``` | returns iterators to the first and one-past-the-last set bits (npos if none) |
| ```cbegin```, ```cend``` | returns const iterators to the first and one-past-the-last set bits (npos if none) |

## Correctness
```hibitset``` has been fuzz-tested with the ```std::set``` as an oracle, along with a set of unit tests that cover the API. The tests can be run by executing:
```sh
cmake -B build-debug -S .
cmake --build build-debug -j
ctest --test-dir "build-debug" --output-on-failure
```

## Behind the scenes
```
Example with U = 2^8, chunk_size = 4:
Layer 3: 1------------------------------------------------ ...
Layer 2: 1------------------ 1------------------ 0-------- ...
Layer 1: 1--- 0--- 0--- 0--- 1--- 0--- 1--- 0--- 0--- 0--- ...
Layer 0: 0010 0000 0000 0000 0011 0000 1111 0000 0000 0000 ...
```

In the real implementation each chunk is a ```uint64_t``` (64 bits) (the diagram above shrinks that to 4 bits per chunk for demonstration). The base layer (layer 0) holds the actual bits, with set bits at indices $\{1, 16, 17, 24, 25, 26, 27\}$. Each layer above summarizes the layer below it:
- Layer 1's bit $i$ is 1 iff any of layer 0's bits in chunk $i$ are set.
- Layer 2's bit $i$ is 1 iff any of layer 1's bits in chunk $i$ are set.
- Layer 3's bit $i$ is 1 iff any of layer 2's bits in chunk $i$ are set.

When we call ```next_set(2)```, the algorithm checks chunk 0 in layer 0 for a set bit at or after index 2. Since chunk 0's only set bit is index 1, it ascends to layer 1 looking past chunk 0's position, but bits 1–3 are all 0, so it ascends again to layer 2's bit 0, which only covers that same empty span. Moving to layer 2's bit 1, it finds a 1, descends into layer 1's bits 4–7, finds bit 4 set, descends once more into layer 0's chunk 4 (```0011```), and returns its lowest set bit, index 16. Instead of scanning every chunk in between, the algorithm skipped straight from chunk 0 to chunk 4 by climbing to the layer where that gap is summarized by a single 0.

For a real $U$ with 64-bit chunks, this ascend/descend pattern spans at most $\log_{64} U$ layers, which is why the worst case for ```next_set```/```prev_set``` is $O(\log_{64} U)$ regardless of how sparse or clustered the set bits are.
