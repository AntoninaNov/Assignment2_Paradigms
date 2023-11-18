cmake_minimum_required(VERSION 3.26)
project(Assignment2_Paradigms)

set(CMAKE_CXX_STANDARD 17)

add_executable(Assignment2_Paradigms main.cpp
        FileReader.h
        FileWriter.h
        CaesarCipher.h)

target_link_libraries(Assignment2_Paradigms "/Users/antoninanovak/CLionProjects/Assignment3_Paradigms/cmake-build-debug/libcaesar.dylib")

