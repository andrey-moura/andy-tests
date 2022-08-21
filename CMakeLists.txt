#Require a minimum version
cmake_minimum_required(VERSION 3.20)

project(uva-cspec)

# Minimun C++ version
set(CMAKE_CXX_STANDARD 20)

add_library(uva-cspec
	${CMAKE_CURRENT_LIST_DIR}/src/cspec.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})

function(cspec_configure project_name source_dir libraries)

	message(STATUS "Initializing tests: ${project_name}-cspec")

	set(tests_folder "${source_dir}/cspec")
	message(STATUS "Tests folder: ${tests_folder}")

	file(GLOB_RECURSE test_files CONFIGURE_DEPENDS 
		"${tests_folder}/*_spec.hpp"
		"${tests_folder}/*_spec.cpp"
	)

	list (APPEND test_files "${tests_folder}/cspec.cpp")

	message(STATUS "Test files: ${test_files}")

	#create tests for parent project
	add_executable(${project_name}-cspec ${test_files})

	#link this library with parent's tests project
	target_link_libraries(${project_name}-cspec uva-cspec ${libraries})

	get_target_property(OUT ${project_name}-cspec LINK_LIBRARIES)
	message(STATUS "Libraries linked to tests: ${OUT}")

	set(cspec_folder "${source_dir}/cspec")
	set(cspec_temp_dir "${cspec_folder}/temp")

	target_compile_definitions(${project_name}-cspec PUBLIC -DCSPEC_FOLDER="${cspec_folder}" -DCSPEC_TEMP_FOLDER="${cspec_temp_dir}")
endfunction()