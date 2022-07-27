#Require a minimum version
cmake_minimum_required(VERSION 3.10)

project(uva-cspec)

# Minimun C++ version
set(CMAKE_CXX_STANDARD 23)

add_library(uva-cspec
	${CMAKE_CURRENT_LIST_DIR}/src/cspec.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})

function(cspec_configure project_name source_dir libraries)

	message(STATUS "Initializing tests: ${project_name}-cspec")
	message(STATUS "Tests folder: ${source_dir}/cspec")

	#create tests for parent project
	add_executable(${project_name}-cspec ${source_dir}/cspec/cspec.cpp)

	#link this library with parent's tests project
	target_link_libraries(${project_name}-cspec uva-cspec ${libraries})

	target_compile_definitions(${project_name}-cspec PUBLIC -DCSPEC_FOLDER="${source_dir}/cspec")
endfunction()