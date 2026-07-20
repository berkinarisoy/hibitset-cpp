include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

add_executable(hibitset_tests
    test_include.cpp
    test_constexpr.cpp
    test_oracle.cpp
)
target_link_libraries(hibitset_tests PRIVATE hibitset::hibitset Catch2::Catch2WithMain)

# test_constexpr.cpp evaluates several U=262144 bitsets at compile time, which
# exceeds the default constexpr evaluation budget on some compilers.
target_compile_options(hibitset_tests PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fconstexpr-steps=8000000>
    $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=100000000>
    $<$<CXX_COMPILER_ID:MSVC>:/constexpr:steps8000000>
)

include(Catch)
catch_discover_tests(hibitset_tests)
