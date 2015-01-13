cmake_minimum_required(VERSION 2.6)
project(nflame)

add_executable(nflame main.cpp)

if(NOT MSVC)
    target_compile_options(nflame PUBLIC -std=c++11)
endif()

install(TARGETS nflame RUNTIME DESTINATION bin)
