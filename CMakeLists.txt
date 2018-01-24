cmake_minimum_required(VERSION 3.9)
project(linux_lowlevel_tracker)

find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread filesystem)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR} ~/libs/efsw/include/efsw/)

find_package(Threads)
set(CMAKE_CXX_STANDARD 11)

add_executable(linux_lowlevel_tracker main.cpp UpdateListener.h)
target_link_libraries(linux_lowlevel_tracker ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} /home/yevheniishc/libs/efsw/lib/libefsw.so)