cmake_minimum_required(VERSION 3.10)

project(HArray LANGUAGES CXX)

add_library(harray STATIC
  src/HArray_delValueByKey.cpp
  src/HArray_getKeysAndValuesByRange.cpp
  src/HArray_getValueByKey.cpp
  src/HArray_hasPartKey.cpp
  src/HArray_insert.cpp
  src/HArray_insertOrGet.cpp
  src/HArray_rebuild.cpp
  src/HArray_scanKeysAndValues.cpp
  src/HArray_shrink.cpp
  src/HArray_test.cpp)

target_include_directories(harray PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/harray)

add_executable(HArrayBenchmark benchmark/HArrayBenchmark.cpp)
target_link_libraries(HArrayBenchmark harray)
