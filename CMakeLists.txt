cmake_minimum_required(VERSION 3.20.0)
project(efft)

find_package(Eigen3 3.4.0 REQUIRED)

include(GoogleTest)
enable_testing()
add_executable(efft-unit-tests ${PROJECT_SOURCE_DIR}/tests/efft.cpp)
target_link_libraries(efft-unit-tests PRIVATE gtest gtest_main pthread)
target_link_libraries(efft-unit-tests PRIVATE fftw3)
target_include_directories(efft-unit-tests PRIVATE include)
target_compile_definitions(efft-unit-tests PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_definitions(efft-unit-tests PRIVATE EFFT_USE_FFTW3)
gtest_discover_tests(efft-unit-tests)
