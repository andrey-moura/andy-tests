#Require a minimum version
cmake_minimum_required(VERSION 3.10)

project(andy-test)

# C++ version
set(CMAKE_CXX_STANDARD 20)

add_library(andy-tests-main ${CMAKE_CURRENT_LIST_DIR}/src/test-main.cpp)
target_link_libraries(andy-tests-main uva-console)
add_executable(andy-test ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

function(andy_tests_configure target)
    message(STATUS "Configuring tests for ${target}")

    message(STATUS "Project source dir: ${CMAKE_CURRENT_LIST_DIR}")
    
    set(ANDY_TESTS_DIR ${CMAKE_CURRENT_LIST_DIR}/tests)
    message(STATUS "Tests source dir: ${ANDY_TESTS_DIR}")

    set(ANDY_TESTS_TARGET_DIR ${ANDY_TESTS_DIR}/${target})
    message(STATUS "Tests target dir: ${ANDY_TESTS_TARGET_DIR}")

    file(GLOB_RECURSE ANDY_TESTS_SOURCES ${ANDY_TESTS_TARGET_DIR}/*spec.cpp)

    foreach(test_source ${ANDY_TESTS_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        message(STATUS "Adding test: ${test_name}")
        
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} andy-tests-main)

        # Check if the target is a library or an executable
        get_target_property(target_type ${target} TYPE)

        if(${target_type} STREQUAL "STATIC_LIBRARY" OR ${target_type} STREQUAL "OBJECT_LIBRARY")
            target_link_libraries(${test_name} ${target})
        endif()

    endforeach()
    
endfunction()