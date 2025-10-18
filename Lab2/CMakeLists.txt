cmake_minimum_required(VERSION 3.10)
project(lab2)

file(GLOB SOURCES "src/*.c")
file(GLOB HEADERS "include/*.h")

add_executable(lab2 ${SOURCES} ${HEADERS})

target_include_directories(lab2 PRIVATE include)

target_link_libraries(lab2 m pthread)

target_compile_options(lab2 PRIVATE -Wall -O2)