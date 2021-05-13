cmake_minimum_required(VERSION 3.16)

project(riehm_nlp_project VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
	include 
)

add_executable(model
	src/main.cpp
	src/string.cpp
	src/custom_net.cpp
	src/ngram.cpp
	src/progress.cpp
	src/node.cpp
	src/input_node.cpp
	src/combo_node.cpp
	src/util.cpp
)