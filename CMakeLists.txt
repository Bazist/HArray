cmake_minimum_required(VERSION 3.10)

project(HArray)

add_executable(HArray
	src/HArray_delValueByKey.cpp
	src/HArray_rebuild.cpp
	src/HArray_getKeysAndValuesByRange.cpp
	src/HArray_scanKeysAndValues.cpp
	src/HArray_getValueByKey.cpp
	src/HArray_shrink.cpp
	src/HArray_hasPartKey.cpp
	src/HArray_test.cpp
	src/HArray_insert.cpp
	src/Main.cpp)

install(TARGETS HArray)

enable_testing()
add_test(NAME HArray COMMAND HArray)