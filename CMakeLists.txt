#project(DFA CXX)
#set(CMAKE_BUILD_TYPE Debug)
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
##set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
##find_package(antlr4-runtime REQUIRED)
#find_package(z3 REQUIRED)
#
#file(GLOB SRC_FILES src/*.cpp src/gen/*.cpp)
#file(GLOB HPP_FILES src/*.hpp src/gen/*.hpp)
#
#add_executable(rr rr.cpp ${SRC_FILES} ${HPP_FILES})
#target_link_libraries(rr PRIVATE z3::libz3)
#
##function(add_target name source_file)
##    add_executable(${name} ${source_file} ${SRC_FILES} ${HPP_FILES})
#    #    target_include_directories(${name} PRIVATE /usr/local/include)
#    #    target_link_directories(${name} PRIVATE /usr/local/lib)
#    #    target_link_libraries(${name} PRIVATE z3)
#    #    target_include_directories(${name} PRIVATE /usr/local/include/antlr4-runtime)
#    #    target_link_libraries(${name} PRIVATE antlr4-runtime)
##    target_link_libraries(${name} PRIVATE antlr4_static)
##    target_link_libraries(${name} PRIVATE z3::libz3)
##endfunction()
#
#
##add_target(rr rr.cpp)
##add_target(prio prio.cpp)
##add_target(loom_mem loom.cpp)
##add_target(fq fq.cpp)
##add_target(leaf leaf.cpp)

cmake_minimum_required(VERSION 3.15)
project(CountBuffy CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Z3 REQUIRED)
find_package(antlr4-runtime REQUIRED)

file(GLOB SRC_FILES src/*.cpp src/gen/*.cpp)
file(GLOB HPP_FILES src/*.hpp src/gen/*.hpp)
file(GLOB EXAMPLE_FILES examples/*.cpp examples/*.hpp)


function(add_target name source_file)
    add_executable(${name} ${source_file} ${SRC_FILES} ${HPP_FILES})
    target_link_libraries(${name} PRIVATE z3::libz3)
    target_link_libraries(${name} PRIVATE antlr4_static)
##    add_executable(${name} ${source_file} ${SRC_FILES} ${HPP_FILES})
#    #    target_include_directories(${name} PRIVATE /usr/local/include)
#    #    target_link_directories(${name} PRIVATE /usr/local/lib)
#    #    target_link_libraries(${name} PRIVATE z3)
#    #    target_include_directories(${name} PRIVATE /usr/local/include/antlr4-runtime)
#    #    target_link_libraries(${name} PRIVATE antlr4-runtime)
##    target_link_libraries(${name} PRIVATE antlr4_static)
##    target_link_libraries(${name} PRIVATE z3::libz3)
endfunction()

add_target(fq fq.cpp)
add_target(loom loom.cpp)
add_target(prio prio.cpp)
add_target(rr rr.cpp)

add_executable(bar ${EXAMPLE_FILES} ${SRC_FILES} ${HPP_FILES})
target_link_libraries(bar PRIVATE z3::libz3)
target_link_libraries(bar PRIVATE antlr4_static)

#add_executable(hello_world prio.cpp ${SRC_FILES} )
#target_compile_options(hello_world PRIVATE -g)
#target_link_libraries(hello_world PRIVATE z3::libz3)
#target_link_libraries(hello_world PRIVATE antlr4_static)
