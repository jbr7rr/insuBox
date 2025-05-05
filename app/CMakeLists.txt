cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C CXX)

target_compile_options(app PRIVATE -Wall -Werror)

target_sources(app PRIVATE src/main.cpp)
