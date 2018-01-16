cmake_minimum_required(VERSION 3.9)
project(linux_lowlevel_tracker)

find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package(Threads)
set(CMAKE_CXX_STANDARD 14)

add_executable(linux_lowlevel_tracker main.cpp)
target_link_libraries(linux_lowlevel_tracker ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})