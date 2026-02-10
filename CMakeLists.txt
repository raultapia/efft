cmake_minimum_required(VERSION 3.20.0)
project(efft LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(benchmark REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET "fftw3>=3.3.8")

if(Eigen3_VERSION VERSION_LESS 3.4.0)
  message(FATAL_ERROR "Eigen3 >= 3.4.0 required, found ${Eigen3_VERSION}")
endif()

include(GoogleTest)
enable_testing()
add_executable(efft-tests ${PROJECT_SOURCE_DIR}/tests/efft_test.cpp)
target_link_libraries(efft-tests PRIVATE GTest::gtest GTest::gtest_main Threads::Threads)
target_link_libraries(efft-tests PRIVATE Eigen3::Eigen)
target_link_libraries(efft-tests PRIVATE PkgConfig::FFTW3)
target_include_directories(efft-tests PRIVATE include)
target_compile_definitions(efft-tests PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_definitions(efft-tests PRIVATE EFFT_USE_FFTW3)
target_compile_definitions(efft-tests PRIVATE NDEBUG)
gtest_discover_tests(efft-tests)
target_compile_options(
  efft-tests
  PRIVATE -O3
          -march=native
          -flto
          -funroll-loops
          -finline-functions
          -fomit-frame-pointer
          -ffast-math
          -Wall
          -Wextra
          -Wpedantic
          -Wfatal-errors)
target_link_options(efft-tests PRIVATE -flto)

add_executable(efft-benchmarks ${PROJECT_SOURCE_DIR}/benchmarks/efft_benchmark.cpp)
target_link_libraries(efft-benchmarks PRIVATE benchmark::benchmark Threads::Threads)
target_link_libraries(efft-benchmarks PRIVATE Eigen3::Eigen)
target_link_libraries(efft-benchmarks PRIVATE PkgConfig::FFTW3)
target_include_directories(efft-benchmarks PRIVATE include)
target_compile_definitions(efft-benchmarks PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_definitions(efft-benchmarks PRIVATE EFFT_USE_FFTW3)
target_compile_definitions(efft-benchmarks PRIVATE NDEBUG)
target_compile_options(
  efft-benchmarks
  PRIVATE -O3
          -march=native
          -flto
          -funroll-loops
          -finline-functions
          -fomit-frame-pointer
          -ffast-math
          -Wall
          -Wextra
          -Wpedantic
          -Wfatal-errors)
target_link_options(efft-benchmarks PRIVATE -flto)
add_custom_target(
  run-benchmark
  COMMAND efft-benchmarks
  DEPENDS efft-benchmarks
  COMMENT "Running eFFT benchmarks")
