#Require a minimum version
cmake_minimum_required(VERSION 3.10)

project(cspec)

include("${UVA_ROOT_DIR}/include/console/console.cmake")

add_console_app(cspec)

target_link_libraries(cspec uva-console uva-core uva-routing uva-json)

add_custom_command(TARGET cspec 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cspec> ${CMAKE_SOURCE_DIR}/bin/cspec)

