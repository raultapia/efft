cmake_minimum_required(VERSION 3.18)
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

find_package(
  Python 3.8
  COMPONENTS Interpreter Development.Module
  REQUIRED)

find_package(Eigen3 3.4.0 REQUIRED)

nanobind_add_module(efft src/efft/bindings.cpp)
target_link_libraries(efft PRIVATE Eigen3::Eigen)
target_include_directories(efft PRIVATE ${CMAKE_SOURCE_DIR}/../include)
target_compile_definitions(efft PRIVATE EIGEN_STACK_ALLOCATION_LIMIT=0)
target_compile_options(
  efft
  PRIVATE -O3
          -march=native
          -flto
          -funroll-loops
          -finline-functions
          -fomit-frame-pointer
          -ffast-math)

install(TARGETS efft LIBRARY DESTINATION "${CMAKE_SOURCE_DIR}/src/efft")
