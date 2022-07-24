add_library(uva-cpsec
	${CMAKE_CURRENT_LIST_DIR}/src/cspec.cpp
)

target_link_libraries(uva-cpsec)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CSPEC_SOURCE_FILE ${CMAKE_SOURCE_DIR}/cspec/cspec.cpp)

if(EXISTS ${CSPEC_SOURCE_FILE})
	message(STATUS "Initializing tests: ${CMAKE_PROJECT_NAME}-cspec")

	add_executable(${CMAKE_PROJECT_NAME}-cspec ${CSPEC_SOURCE_FILE})
endif()