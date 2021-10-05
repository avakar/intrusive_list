cmake_minimum_required(VERSION 3.19)
project(avakar.intrusive_list)

include(FetchContent)
FetchContent_Declare(avakar.mutest SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/mutest")
FetchContent_Declare(avakar.container_of SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/container_of")
FetchContent_MakeAvailable(avakar.mutest avakar.container_of)

add_library(avakar.intrusive_list INTERFACE
	include/avakar/intrusive/list.h
	)
add_library(avakar::intrusive_list ALIAS avakar.intrusive_list)

target_include_directories(avakar.intrusive_list INTERFACE include)
target_compile_features(avakar.intrusive_list INTERFACE cxx_std_11)
target_link_libraries(avakar.intrusive_list INTERFACE avakar::container_of)

add_mutest(test/test.cpp avakar::intrusive_list)
