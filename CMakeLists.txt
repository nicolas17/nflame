cmake_minimum_required(VERSION 2.6)
project(nflame)

add_executable(nflame main.cpp)

install(TARGETS nflame RUNTIME DESTINATION bin)
