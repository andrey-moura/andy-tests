#Require a minimum version
cmake_minimum_required(VERSION 3.10)

project(uva-cspec)

# Minimun C++ version
set(CMAKE_CXX_STANDARD 23)

add_library(uva-cspec
	${CMAKE_CURRENT_LIST_DIR}/src/cspec.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})

#top level project should include uva dependencies
if(EXISTS ${CSPEC_SOURCE_FILE})

	set(CSPEC_SOURCE_FOLDER ${CMAKE_SOURCE_DIR}/cspec)
	set(CSPEC_SOURCE_FILE 	${CSPEC_SOURCE_FOLDER}/cspec.cpp)

	message(STATUS "Initializing tests: ${CMAKE_PROJECT_NAME}-cspec")
	message(STATUS "Tests folder: ${CSPEC_SOURCE_FOLDER}")

	#create tests for parent project
	add_executable(${CMAKE_PROJECT_NAME}-cspec ${CSPEC_SOURCE_FILE})

	#link this library with parent's tests project
	target_link_libraries(${CMAKE_PROJECT_NAME}-cspec uva-cspec)

	target_compile_definitions(${CMAKE_PROJECT_NAME}-cspec PUBLIC -DCSPEC_FOLDER=${CSPEC_SOURCE_FILE})

endif()