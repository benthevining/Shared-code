include_guard (GLOBAL)

cmake_minimum_required (VERSION 3.19)

project (bv_SharedCode VERSION 0.0.1 LANGUAGES CXX C)

set (BV_CXX_VERSION 20 CACHE INTERNAL "C++ standard version")

add_subdirectory (cmake)
add_subdirectory (modules)
