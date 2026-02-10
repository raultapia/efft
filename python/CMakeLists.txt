cmake_minimum_required(VERSION 3.20.0)
project(efft LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  nanobind
  GIT_REPOSITORY https://github.com/wjakob/nanobind.git
  GIT_TAG v2.2.0)
FetchContent_MakeAvailable(nanobind)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)
FetchContent_MakeAvailable(eigen)

find_package(
  Python 3.8
  COMPONENTS Interpreter Development.Module
  REQUIRED)

nanobind_add_module(_efft src/efft/bindings.cpp)
target_link_libraries(_efft PRIVATE Eigen3::Eigen)
target_include_directories(_efft PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_definitions(_efft PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_options(
  _efft
  PRIVATE -O3
          -march=native
          -flto
          -funroll-loops
          -finline-functions
          -fomit-frame-pointer
          -ffast-math)

install(
  TARGETS _efft
  LIBRARY DESTINATION efft
  RUNTIME DESTINATION efft
  ARCHIVE DESTINATION efft)
