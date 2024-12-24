#Require a minimum version
cmake_minimum_required(VERSION 3.10)

project(uva-test)

# C++ version
set(CMAKE_CXX_STANDARD 20)

add_library(uva-tests-main ${CMAKE_CURRENT_LIST_DIR}/src/test-main.cpp)
target_link_libraries(uva-tests-main uva-console)
add_executable(uva-test ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

function(uva_tests_configure target)
    message(STATUS "Configuring tests for ${target}")

    message(STATUS "Project source dir: ${CMAKE_CURRENT_LIST_DIR}")
    
    set(UVA_TESTS_DIR ${CMAKE_CURRENT_LIST_DIR}/uva-tests)
    message(STATUS "Tests source dir: ${UVA_TESTS_DIR}")

    set(UVA_TESTS_TARGET_DIR ${UVA_TESTS_DIR}/${target})
    message(STATUS "Tests target dir: ${UVA_TESTS_TARGET_DIR}")

    file(GLOB_RECURSE UVA_TESTS_SOURCES ${UVA_TESTS_TARGET_DIR}/*spec.cpp)

    foreach(test_source ${UVA_TESTS_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        message(STATUS "Adding test: ${test_name}")
        
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} uva-tests-main)

        # Check if the target is a library or an executable
        get_target_property(target_type ${target} TYPE)

        if(${target_type} STREQUAL "STATIC_LIBRARY")
            target_link_libraries(${test_name} ${target})
        endif()

    endforeach()
    
endfunction()