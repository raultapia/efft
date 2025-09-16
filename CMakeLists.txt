cmake_minimum_required(VERSION 3.20.0)
project(efft)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 3.4.0 REQUIRED)

include(GoogleTest)
enable_testing()
add_executable(efft-tests ${PROJECT_SOURCE_DIR}/tests/efft_test.cpp)
target_link_libraries(efft-tests PRIVATE gtest gtest_main pthread)
target_link_libraries(efft-tests PRIVATE fftw3)
target_include_directories(efft-tests PRIVATE include)
target_compile_definitions(efft-tests PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_definitions(efft-tests PRIVATE EFFT_USE_FFTW3)
gtest_discover_tests(efft-tests)

find_package(benchmark)
add_executable(efft-benchmarks ${PROJECT_SOURCE_DIR}/benchmarks/efft_benchmark.cpp)
target_link_libraries(efft-benchmarks PRIVATE benchmark::benchmark pthread)
target_link_libraries(efft-benchmarks PRIVATE fftw3)
target_include_directories(efft-benchmarks PRIVATE include)
target_compile_definitions(efft-benchmarks PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_definitions(efft-benchmarks PRIVATE EFFT_USE_FFTW3)
add_custom_target(
  benchmark
  COMMAND efft-benchmarks
  DEPENDS efft-benchmarks
  COMMENT "Running eFFT benchmarks")
