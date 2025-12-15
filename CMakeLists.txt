cmake_minimum_required(VERSION 3.15)
project(CountBuffy CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Z3 REQUIRED)
find_package(antlr4-runtime REQUIRED)

file(GLOB SRC_FILES src/*.cpp src/gen/*.cpp)
file(GLOB HPP_FILES src/*.hpp src/gen/*.hpp)
file(GLOB EXAMPLE_FILES examples/*.cpp examples/*.hpp)


function(add_target name source_file)
    add_executable(${name} ${source_file} ${SRC_FILES} ${HPP_FILES})
    target_link_libraries(${name} PRIVATE z3::libz3 antlr4_static)
endfunction()

add_target(fq fq.cpp)
add_target(loom loom.cpp)
add_target(prio prio.cpp)
add_target(rr rr.cpp)

add_executable(hello-world ${EXAMPLE_FILES} ${SRC_FILES} ${HPP_FILES})
target_link_libraries(hello-world PRIVATE z3::libz3 antlr4_static)
