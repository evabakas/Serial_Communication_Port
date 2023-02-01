# CMake main config file to build the project #
# Author: Vangelis Bakas 

# Version and project name 
cmake_minimum_required(VERSION 3.10)
project(SerialComm VERSION 1.0 LANGUAGES C)

# Set the two executables, i.e. server and client
add_executable(server server.c)
add_executable(client client.c)

# Add the library for common structures and functions 
add_library(commonfunc commonfunc.h commonfunc.c)

# Link the library to the executables 
target_link_libraries(server PUBLIC commonfunc)
target_link_libraries(client PUBLIC commonfunc)