cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(DFA)
file(GLOB SRC_FILES src/*.cpp)
file(GLOB HPP_FILES src/*.hpp)

add_executable(Single main.cpp ${SRC_FILES} ${HPP_FILES})
target_include_directories(Single PRIVATE /usr/local/include)
target_link_directories(Single PRIVATE /usr/local/lib)
target_link_libraries(Single PRIVATE z3)

add_executable(Comp comp.cpp ${SRC_FILES} ${HPP_FILES})
target_include_directories(Comp PRIVATE /usr/local/include)
target_link_directories(Comp PRIVATE /usr/local/lib)
target_link_libraries(Comp PRIVATE z3)

add_executable(Wins wins.cpp ${SRC_FILES} ${HPP_FILES})
target_include_directories(Wins PRIVATE /usr/local/include)
target_link_directories(Wins PRIVATE /usr/local/lib)
target_link_libraries(Wins PRIVATE z3)

add_executable(Debug debug.cpp ${SRC_FILES} ${HPP_FILES})
target_include_directories(Debug PRIVATE /usr/local/include)
target_link_directories(Debug PRIVATE /usr/local/lib)
target_link_libraries(Debug PRIVATE z3)

