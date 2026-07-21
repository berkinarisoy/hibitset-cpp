include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1
)
FetchContent_MakeAvailable(benchmark)

find_package(Boost REQUIRED COMPONENTS headers)
get_target_property(Boost_INCLUDE_DIRS Boost::headers INTERFACE_INCLUDE_DIRECTORIES)

add_executable(hibitset_bench
    bench_queries.cpp
    bench_mutations.cpp
    bench_iteration.cpp
    bench_algebra.cpp
    bench_construction.cpp
    bench_memory.cpp
)
target_link_libraries(hibitset_bench PRIVATE hibitset::hibitset benchmark::benchmark_main Boost::headers)
target_include_directories(hibitset_bench PRIVATE ${Boost_INCLUDE_DIRS})