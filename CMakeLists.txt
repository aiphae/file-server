cmake_minimum_required(VERSION 3.10)

project(file-server)

add_executable(file-server server.c)

target_link_libraries(file-server ws2_32)
